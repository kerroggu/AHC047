#include <bits/stdc++.h>
using namespace std;

const double TL = 1.95;
static const double START_TEMP = 10.0;
static const double END_TEMP = 0.0001;
static std::mt19937 rng(123456789);
static inline double rand_double(){ return std::uniform_real_distribution<double>(0.0,1.0)(rng); }
static inline int rand_int(int l,int r){ return std::uniform_int_distribution<int>(l,r)(rng); }


// Build a 6-state automaton for a single string.
// `start` is the state index offset for this string.
// `other_first` is the first state index of the other string.
static void build(const string& s, int start, int other_first,
                  vector<char>& C, vector<vector<int>>& A) {
    const int M = C.size();

    // Collect unique letters used in the string.
    vector<char> letters;
    vector<int> seen(6, 0);
    for (char ch : s) {
        int id = ch - 'a';
        if (!seen[id]) {
            letters.push_back(ch);
            seen[id] = 1;
        }
    }
    if (letters.empty()) letters.push_back('a');
    int K = letters.size();

    // Assign 6 states by repeating the used letters.
    vector<char> states(6);
    for (int i = 0; i < 6; i++) {
        states[i] = letters[i % K];
        C[start + i] = states[i];
    }

    // Map each letter to the first corresponding state index.
    vector<int> char_to_state(6, -1);
    for (int i = 0; i < 6; i++) {
        int id = states[i] - 'a';
        if (char_to_state[id] == -1) char_to_state[id] = start + i;
    }

    // Determine possible next letters for each letter (cyclic).
    vector<set<char>> nxt(6);
    for (size_t i = 0; i < s.size(); i++) {
        char cur = s[i];
        char nx = s[(i + 1) % s.size()];
        nxt[cur - 'a'].insert(nx);
    }

    for (int i = 0; i < 6; i++) {
        int idx = start + i;
        vector<int> row(M, 0);

        // 1% chance to jump to the beginning of the other string.
        row[other_first] = 1;
        int remaining = 99; // remaining probability mass

        char c = states[i];
        vector<char> options(nxt[c - 'a'].begin(), nxt[c - 'a'].end());
        int k = options.size();
        int prob = 0;
        if (k == 1) prob = 41;
        else if (k == 2) prob = 41;
        else if (k == 3) prob = 33;
        else if (k > 0) prob = 99 / k; // fallback for rare cases
        for (int t = 0; t < k; t++) {
            int dest = char_to_state[options[t] - 'a'];
            if (dest == -1) dest = start; // fallback
            row[dest] += prob;
            remaining -= prob;
        }

        // Distribute the rest evenly among the other states excluding the chosen destinations.
        vector<int> dest_idx;
        for (char ch : options) {
            int d = char_to_state[ch - 'a'];
            if (d != -1) dest_idx.push_back(d);
        }
        vector<int> others;
        for (int j = 0; j < 6; j++) {
            int id = start + j;
            if (j == i) continue;
            if (find(dest_idx.begin(), dest_idx.end(), id) != dest_idx.end()) continue;
            others.push_back(id);
        }
        if (others.empty()) others.push_back(start); // fallback
        int m = others.size();
        for (int t = 0; t < m; t++) {
            row[others[t]] += remaining / m;
            if (t < remaining % m) row[others[t]] += 1;
        }

        A[idx] = row;
    }
}

// Build a 12-state automaton using only one string.
static void build_single(const string& s, vector<char>& C,
                         vector<vector<int>>& A) {
    int M = C.size();
    for (int i = 0; i < M; i++) {
        C[i] = s[i % (int)s.size()];
    }
    for (int i = 0; i < M; i++) {
        vector<int> row(M, 0);
        int next = (i + 1) % M;
        row[next] = 41;
        int rem = 59;
        int cnt = M - 1;
        for (int j = 0; j < M; j++) {
            if (j == next) continue;
            row[j] += rem / cnt;
        }
        int extra = rem % cnt;
        for (int j = 0; j < M && extra > 0; j++) {
            if (j == next) continue;
            row[j]++;
            extra--;
        }
        A[i] = row;
    }
}

// Build a 12-state automaton using the top `k` strings.
// The first string forms the base 41% path. Subsequent strings
// try to add their paths with 41% transitions on unused edges.
static void build_multi_k(const vector<string>& S, const vector<int>& ord,
                          int k, vector<char>& C, vector<vector<int>>& A) {
    int M = C.size();
    string base = S[ord[0]];
    for (int i = 0; i < M; i++) {
        C[i] = base[i % (int)base.size()];
    }

    vector<vector<int>> B(M, vector<int>(M, -1));
    for (int i = 0; i < M; i++) {
        int nxt = (i + 1) % M;
        B[i][nxt] = 41;
    }

    int use = min(k, (int)S.size());
    for (int t = 1; t < use; t++) {
        const string& str = S[ord[t]];
        int L = str.size();
        for (int i = 0; i < L; i++) {
            char cur = str[i];
            char nx = str[(i + 1) % L];
            vector<int> froms, tos;
            for (int j = 0; j < M; j++) if (C[j] == cur) froms.push_back(j);
            for (int j = 0; j < M; j++) if (C[j] == nx) tos.push_back(j);
            bool done = false;
            for (int fr : froms) {
                int sum = 0;
                for (int j = 0; j < M; j++) if (B[fr][j] != -1) sum += B[fr][j];
                if (sum + 41 > 100) continue;
                for (int to : tos) {
                    if (B[fr][to] == -1) {
                        B[fr][to] = 41;
                        done = true;
                        break;
                    }
                }
                if (done) break;
            }
        }
    }

    for (int i = 0; i < M; i++) {
        vector<int> row(M, 0);
        int sum = 0;
        vector<int> others;
        for (int j = 0; j < M; j++) {
            if (B[i][j] != -1) {
                row[j] = B[i][j];
                sum += row[j];
            } else {
                others.push_back(j);
            }
        }
        int rem = 100 - sum;
        int m = others.size();
        if (m == 0) {
            if (rem > 0) row[(i + 1) % M] += rem;
        } else {
            for (int idx = 0; idx < m; idx++) row[others[idx]] = rem / m;
            for (int idx = 0; idx < rem % m; idx++) row[others[idx]] += 1;
        }
        A[i] = row;
    }
}

// Build a 12-state automaton by allocating 6 states for the top string and
// gradually adding paths of the next (k-1) strings. When a 41% transition cannot
// be inserted due to probability constraints, a new state is created as long as
// unused states remain.
static void build_multi_k_seq(const vector<string>& S, const vector<int>& ord,
                              int k, vector<char>& C, vector<vector<int>>& A) {
    const int M = C.size();
    const string& base = S[ord[0]];

    // number of states currently used
    int used = 0;
    vector<vector<int>> B(M, vector<int>(M, -1));

    // assign first 6 states to the top string
    for (int i = 0; i < 6 && i < M; i++) {
        C[i] = base[i % (int)base.size()];
        used++;
    }

    // base 41% path within the first 6 states
    for (int i = 0; i < used; i++) {
        int to = (i + 1) % used;
        B[i][to] = 41;
    }

    int use = min(k, (int)S.size());
    for (int t = 1; t < use; t++) {
        const string& str = S[ord[t]];
        int L = str.size();
        for (int i = 0; i < L; i++) {
            char cur = str[i];
            char nx = str[(i + 1) % L];

            // search state for `cur` that can accept a new 41% transition
            int fr = -1;
            for (int j = 0; j < used; j++) {
                if (C[j] != cur) continue;
                int sum = 0;
                for (int x = 0; x < used; x++) if (B[j][x] != -1) sum += B[j][x];
                if (sum + 41 <= 100) { fr = j; break; }
            }
            if (fr == -1 && used < M) {
                fr = used;
                C[used++] = cur;
            }
            if (fr == -1) continue;

            // search destination state for `nx`
            int to = -1;
            for (int j = 0; j < used; j++) {
                if (C[j] == nx && B[fr][j] == -1) { to = j; break; }
            }
            if (to == -1 && used < M) {
                to = used;
                C[used++] = nx;
            }
            if (to == -1) continue;

            B[fr][to] = 41;
        }
    }

    // fill remaining characters for unused states
    for (int i = used; i < M; i++) {
        C[i] = base[i % (int)base.size()];
    }

    // finalize transition probabilities
    for (int i = 0; i < M; i++) {
        vector<int> row(M, 0);
        int sum = 0;
        for (int j = 0; j < M; j++) {
            if (B[i][j] != -1) {
                row[j] = B[i][j];
                sum += row[j];
            }
        }
        int rem = 100 - sum;
        vector<int> others;
        for (int j = 0; j < M; j++) {
            if (j == i) continue;
            if (row[j] == 0) others.push_back(j);
        }
        if (others.empty()) {
            row[(i + 1) % M] += rem;
        } else {
            int m = others.size();
            for (int idx = 0; idx < m; idx++) row[others[idx]] += rem / m;
            for (int idx = 0; idx < rem % m; idx++) row[others[idx]] += 1;
        }
        A[i] = row;
    }
}

// Embed the top `k` strings sequentially. For each consecutive pair `x`,`y` in
// a string, search for a state representing `x`. If it does not exist, create a
// new state if possible and connect it to `y`. If `x -> y` already exists, move
// on. If `x` has fewer than two outgoing edges, add the transition. Otherwise,
// create a new state for `x` and add the edge if states remain. All inserted
// edges get probability `prob` (default 41). Remaining probabilities are
// distributed uniformly.
static void build_embed_k(const vector<string>& S, const vector<int>& ord,
                          int k, int prob, vector<char>& C,
                          vector<vector<int>>& A) {
    const int M = C.size();
    vector<vector<int>> B(M, vector<int>(M, -1));
    vector<char> letters(M, '?');

    int used = 0; // number of defined states

    auto find_state = [&](char ch) -> int {
        for (int i = 0; i < used; i++) if (letters[i] == ch) return i;
        return -1;
    };
    auto add_state = [&](char ch) -> int {
        if (used >= M) return -1;
        letters[used] = ch;
        return used++;
    };

    int use = min(k, (int)S.size());
    for (int t = 0; t < use; t++) {
        const string& str = S[ord[t]];
        int L = str.size();
        for (int i = 0; i < L; i++) {
            char x = str[i];
            char y = str[(i + 1) % L];

            int sx = find_state(x);
            if (sx == -1) {
                sx = add_state(x);
                if (sx == -1) continue;
            }

            int sy = find_state(y);
            if (sy == -1) {
                sy = add_state(y);
                if (sy == -1) continue;
            }

            if (B[sx][sy] != -1) continue; // already exists

            int out_cnt = 0;
            for (int j = 0; j < M; j++) if (B[sx][j] != -1) out_cnt++;

            if (out_cnt < 2) {
                B[sx][sy] = prob;
            } else if (out_cnt >= 2) {
                int nx = add_state(x);
                if (nx == -1) continue;
                if (B[nx][sy] == -1) B[nx][sy] = prob;
            }
        }
    }

    // fill unused states with characters from the best string
    const string& base = S[ord[0]];
    for (int i = used; i < M; i++) letters[i] = base[i % (int)base.size()];

    // finalize transitions
    for (int i = 0; i < M; i++) {
        vector<int> row(M, 0);
        int sum = 0;
        for (int j = 0; j < M; j++) {
            if (B[i][j] != -1) {
                row[j] = B[i][j];
                sum += row[j];
            }
        }
        int rem = 100 - sum;
        vector<int> others;
        for (int j = 0; j < M; j++) {
            if (row[j] == 0) others.push_back(j);
        }
        if (others.empty()) {
            row[(i + 1) % M] += rem;
        } else {
            int m = others.size();
            for (int t = 0; t < m; t++) row[others[t]] += rem / m;
            for (int t = 0; t < rem % m; t++) row[others[t]] += 1;
        }
        C[i] = letters[i];
        A[i] = row;
    }
}

// Similar to build_embed_k but ensures that each string forms a continuous
// path from its first character to the last. When embedding the next string,
// a transition from the previous string's last state to the new string's first
// state is inserted with probability `prob` if possible.
static void build_ordered_embed_k(const vector<string>& S, const vector<int>& ord,
                                  int k, int prob, vector<char>& C,
                                  vector<vector<int>>& A) {
    const int M = C.size();
    vector<vector<int>> B(M, vector<int>(M, -1));
    vector<char> letters(M, '?');

    int used = 0;
    auto find_state = [&](char ch) -> int {
        for (int i = 0; i < used; i++) if (letters[i] == ch) return i;
        return -1;
    };
    auto add_state = [&](char ch) -> int {
        if (used >= M) return -1;
        letters[used] = ch;
        return used++;
    };
    auto out_cnt = [&](int s) -> int {
        int c = 0;
        for (int j = 0; j < used; j++) if (B[s][j] != -1) c++;
        return c;
    };

    int use = min(k, (int)S.size());
    int prev_last = -1; // last state of the previous string
    for (int t = 0; t < use; t++) {
        const string& str = S[ord[t]];
        int L = str.size();
        if (L == 0) continue;

        int cur = find_state(str[0]);
        if (cur == -1) {
            cur = add_state(str[0]);
            if (cur == -1) break;
        }

        if (prev_last != -1 && B[prev_last][cur] == -1) {
            if (out_cnt(prev_last) < 2) {
                B[prev_last][cur] = prob;
            } else {
                int nx = add_state(letters[prev_last]);
                if (nx != -1) {
                    B[nx][cur] = prob;
                }
            }
        }

        for (int i = 1; i < L; i++) {
            char x = str[i - 1];
            char y = str[i];

            if (letters[cur] != x) {
                int s2 = find_state(x);
                if (s2 == -1) {
                    s2 = add_state(x);
                    if (s2 == -1) break;
                }
                cur = s2;
            }

            int dest = -1;
            for (int j = 0; j < used; j++) {
                if (letters[j] == y && B[cur][j] != -1) { dest = j; break; }
            }
            if (dest == -1) {
                int sy = find_state(y);
                if (sy == -1) {
                    sy = add_state(y);
                    if (sy == -1) break;
                }
                if (B[cur][sy] == -1) {
                    if (out_cnt(cur) < 2) {
                        B[cur][sy] = prob;
                    } else {
                        int nx = add_state(x);
                        if (nx != -1) {
                            B[nx][sy] = prob;
                            cur = nx;
                        }
                    }
                }
                dest = sy;
            }
            cur = dest;
        }
        prev_last = cur;
    }

    const string& base = S[ord[0]];
    for (int i = used; i < M; i++) letters[i] = base[i % (int)base.size()];

    for (int i = 0; i < M; i++) {
        vector<int> row(M, 0);
        int sum = 0;
        for (int j = 0; j < M; j++) {
            if (B[i][j] != -1) {
                row[j] = B[i][j];
                sum += row[j];
            }
        }
        int rem = 100 - sum;
        vector<int> others;
        for (int j = 0; j < M; j++) {
            if (row[j] == 0) others.push_back(j);
        }
        if (others.empty()) {
            row[(i + 1) % M] += rem;
        } else {
            int m = others.size();
            for (int t = 0; t < m; t++) row[others[t]] += rem / m;
            for (int t = 0; t < rem % m; t++) row[others[t]] += 1;
        }
        C[i] = letters[i];
        A[i] = row;
    }
}

// Multiply matrices (n x n) of doubles.
static vector<vector<double>> mul(const vector<vector<double>>& A,
                                  const vector<vector<double>>& B) {
    int n = A.size();
    vector<vector<double>> C(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            if (A[i][k] == 0.0) continue;
            double a = A[i][k];
            for (int j = 0; j < n; j++) {
                C[i][j] += a * B[k][j];
            }
        }
    }
    return C;
}

// Compute X^power for square matrix X.
static vector<vector<double>> mat_pow(vector<vector<double>> base,
                                      long long power) {
    int n = base.size();
    vector<vector<double>> result(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; i++) result[i][i] = 1.0;
    while (power > 0) {
        if (power & 1) result = mul(result, base);
        base = mul(base, base);
        power >>= 1;
    }
    return result;
}

// Probability that `word` appears in length L walk.
static double compute_word_probability(const string& word, long long L,
                                       const vector<char>& C,
                                       const vector<vector<int>>& A) {
    int M = C.size();
    int W = word.size();
    map<pair<int, int>, int> states;
    int n = 0;
    for (int j = 0; j < M; j++) {
        states[{0, j}] = n++;
        for (int i = 0; i < W - 1; i++) {
            if (word[i] == C[j]) {
                states[{i + 1, j}] = n++;
            }
        }
    }
    vector<vector<double>> X(n, vector<double>(n, 0.0));
    for (auto& kv : states) {
        int len = kv.first.first;
        int u = kv.first.second;
        int j = kv.second;
        for (int v = 0; v < M; v++) {
            string next = word.substr(0, len) + C[v];
            int s = 0;
            while (next.substr(s) != word.substr(0, next.size() - s)) s++;
            if ((int)next.size() - s != W) {
                int i = states[{(int)next.size() - s, v}];
                X[i][j] += A[u][v] / 100.0;
            }
        }
    }
    vector<vector<double>> Y;
    if (L <= 1) {
        Y.assign(n, vector<double>(n, 0.0));
        for (int i = 0; i < n; i++) Y[i][i] = 1.0;
    } else {
        Y = mat_pow(X, L - 1);
    }
    int init = (C[0] == word[0]) ? states[{1, 0}] : states[{0, 0}];
    double ret = 1.0;
    for (int i = 0; i < n; i++) ret -= Y[i][init];
    if (ret < 0) ret = 0.0;
    if (ret > 1) ret = 1.0;
    return ret;
}

// Compute total score for the given automaton.
static long long compute_score(const vector<string>& S, const vector<int>& P,
                               long long L, const vector<char>& C,
                               const vector<vector<int>>& A) {
    int M = C.size();
    for (int i = 0; i < M; i++) {
        int sum = accumulate(A[i].begin(), A[i].end(), 0);
        if (sum != 100) return 0;
    }
    double total = 0.0;
    for (size_t i = 0; i < S.size(); i++) {
        double prob = compute_word_probability(S[i], L, C, A);
        total += prob * P[i];
    }
    return llround(total);
}

// Compute stationary distribution of transition matrix.
static vector<double> compute_stationary(const vector<vector<int>>& A) {
    int M = A.size();
    vector<double> pi(M, 1.0 / M), nxt(M);
    for (int iter = 0; iter < 200; iter++) {
        fill(nxt.begin(), nxt.end(), 0.0);
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                nxt[j] += pi[i] * A[i][j] * 0.01;
            }
        }
        double diff = 0.0;
        for (int i = 0; i < M; i++) diff += fabs(nxt[i] - pi[i]);
        pi.swap(nxt);
        if (diff < 1e-12) break;
    }
    return pi;
}

// Probability that `word` appears starting at a random position.
static double approx_word_once(const string& word, const vector<char>& C,
                               const vector<vector<int>>& A,
                               const vector<double>& pi) {
    int M = C.size();
    vector<vector<int>> by(6);
    for (int i = 0; i < M; i++) by[C[i] - 'a'].push_back(i);
    vector<double> dp(M, 0.0);
    for (int s : by[word[0] - 'a']) dp[s] += pi[s];
    for (size_t t = 1; t < word.size(); t++) {
        vector<double> nx(M, 0.0);
        for (int i = 0; i < M; i++) {
            if (dp[i] == 0.0) continue;
            for (int j : by[word[t] - 'a']) {
                nx[j] += dp[i] * A[i][j] * 0.01;
            }
        }
        dp.swap(nx);
    }
    double p = 0.0;
    for (double v : dp) p += v;
    return p;
}

// Fast approximate score using stationary distribution.
static long long fast_score(const vector<string>& S, const vector<int>& P,
                            long long L, const vector<char>& C,
                            const vector<vector<int>>& A) {
    int M = C.size();
    for (int i = 0; i < M; i++) {
        int sum = accumulate(A[i].begin(), A[i].end(), 0);
        if (sum != 100) return 0;
    }
    vector<double> pi = compute_stationary(A);
    double total = 0.0;
    for (size_t i = 0; i < S.size(); i++) {
        double p = approx_word_once(S[i], C, A, pi);
        long long occ = L - (long long)S[i].size() + 1;
        if (occ > 0) {
            double q = 1.0 - pow(1.0 - p, occ);
            if (q < 0.0) q = 0.0;
            if (q > 1.0) q = 1.0;
            total += q * P[i];
        }
    }
    return llround(total);
}

// Convert adjacency lists to transition probabilities.
static void adj_to_matrix(const vector<vector<int>>& adj,
                          vector<vector<int>>& A) {
    int M = adj.size();
    A.assign(M, vector<int>(M, 0));
    for (int i = 0; i < M; i++) {
        int deg = adj[i].size();
        if (deg == 0) {
            A[i][i] = 100;
            continue;
        }
        int per;
        if (deg == 2) per = 41;
        else if (deg == 3) per = 32;
        else if (deg == 4) per = 24;
        else per = 100 / deg;
        int sum = 0;
        for (int to : adj[i]) {
            A[i][to] += per;
            sum += per;
        }
        int rem = 100 - sum;
        for (int j = 0; j < M; j++) {
            A[i][j] += rem / M;
        }
        for (int j = 0; j < rem % M; j++) {
            A[i][j]++;
        }
    }
}

// Build initial 12-state automaton from the top two strings.
// Build initial 12-state automaton with fixed letters "abcdefabcdef" using the
// top two strings. States [0,5] correspond to the first string and [6,11] to
// the second. Transitions try to follow each string while allowing 1% jumps to
// the other string's start.
static void build_fixed_two(const string& s1, const string& s2,
                            vector<char>& C, vector<vector<int>>& A) {
    const string letters = "abcdefabcdef";
    const int M = C.size();
    for (int i = 0; i < M && i < (int)letters.size(); i++) {
        C[i] = letters[i];
    }

    auto assign = [&](const string& s, int start, int other_first) {
        vector<char> states(6);
        for (int i = 0; i < 6; i++) states[i] = C[start + i];
        vector<int> char_to_state(6);
        for (int i = 0; i < 6; i++) char_to_state[states[i] - 'a'] = start + i;

        vector<set<char>> nxt(6);
        for (size_t i = 0; i < s.size(); i++) {
            char cur = s[i];
            char nx = s[(i + 1) % s.size()];
            nxt[cur - 'a'].insert(nx);
        }

        for (int i = 0; i < 6; i++) {
            int idx = start + i;
            vector<int> row(M, 0);

            // 1% chance to jump to the beginning of the other string.
            row[other_first] = 1;
            int remaining = 99;

            char c = states[i];
            vector<char> options(nxt[c - 'a'].begin(), nxt[c - 'a'].end());
            int k = options.size();
            int prob = 0;
            if (k == 1) prob = 41;
            else if (k == 2) prob = 41;
            else if (k == 3) prob = 33;
            else if (k > 0) prob = 99 / k;
            for (int t = 0; t < k; t++) {
                int dest = char_to_state[options[t] - 'a'];
                row[dest] += prob;
                remaining -= prob;
            }

            vector<int> dest_idx;
            for (char ch : options) dest_idx.push_back(char_to_state[ch - 'a']);
            vector<int> others;
            for (int j = 0; j < 6; j++) {
                int id = start + j;
                if (j == i) continue;
                if (find(dest_idx.begin(), dest_idx.end(), id) != dest_idx.end()) continue;
                others.push_back(id);
            }
            if (others.empty()) others.push_back(start);
            int m = others.size();
            for (int t = 0; t < m; t++) {
                row[others[t]] += remaining / m;
                if (t < remaining % m) row[others[t]] += 1;
            }

            A[idx] = row;
        }
    };

    assign(s1, 0, 6);
    assign(s2, 6, 0);
}

// Simulated annealing on transition matrix probabilities.
static void anneal_matrix(const vector<string>& S, const vector<int>& P,
                          long long L, vector<char>& C, vector<vector<int>>& A) {
    int M = C.size();
    vector<vector<int>> curA = A;
    vector<vector<int>> bestA = A;
    long long cur = fast_score(S, P, L, C, curA);
    long long best = cur;

    auto print_solution = [&](const vector<char>& C, const vector<vector<int>>& A) {
        for (int i = 0; i < M; i++) {
            cout << C[i];
            for (int j = 0; j < M; j++) {
                cout << ' ' << A[i][j];
            }
            cout << '\n';
        }
        cout.flush();
    };

    auto start = chrono::steady_clock::now();
    
    int loop = 0;
    while (chrono::duration<double>(chrono::steady_clock::now() - start).count() < TL) {
        ++loop;
        int op = rand_int(0, 1);
        if (op == 0) {
            int i = rand_int(0, M - 1);
            int j1 = rand_int(0, M - 1);
            int j2 = rand_int(0, M - 1);
            if (j1 == j2) continue;
            int delta = rand_int(1, 5);
            double elapsed = chrono::duration<double>(chrono::steady_clock::now() - start).count();
            int lb = (elapsed > TL - 0.5) ? 0 : 1;
            if (curA[i][j1] - delta < lb || curA[i][j2] + delta > 100) continue;
            curA[i][j1] -= delta;
            curA[i][j2] += delta;
            long long sc = fast_score(S, P, L, C, curA);
            double t = START_TEMP * pow(END_TEMP / START_TEMP,
                                        chrono::duration<double>(chrono::steady_clock::now() - start).count() / TL);
            if (sc > cur || rand_double() < exp((double)(sc - cur) / t)) {
                cur = sc;
                if (sc > best) {
                    best = sc;
                    bestA = curA;
                    if (loop >= 1000) {
                        print_solution(C, bestA);
                    }
                }
            } else {
                curA[i][j1] += delta;
                curA[i][j2] -= delta;
            }
        } else {
            int i = rand_int(0, M - 1);
            int j1 = rand_int(0, M - 1);
            int j2 = rand_int(0, M - 1);
            if (j1 == j2 || i == j1 || i == j2) continue;
            int delta = rand_int(1, 5);
            double elapsed = chrono::duration<double>(chrono::steady_clock::now() - start).count();
            int lb = (elapsed > TL - 0.5) ? 0 : 1;
            if (curA[i][j1] - delta < lb || curA[i][j2] + delta > 100) continue;
            if (curA[j2][i] - delta < lb || curA[j1][i] + delta > 100) continue;
            curA[i][j1] -= delta;
            curA[i][j2] += delta;
            curA[j2][i] -= delta;
            curA[j1][i] += delta;
            long long sc = fast_score(S, P, L, C, curA);
            double t = START_TEMP * pow(END_TEMP / START_TEMP,
                                        chrono::duration<double>(chrono::steady_clock::now() - start).count() / TL);
            if (sc > cur || rand_double() < exp((double)(sc - cur) / t)) {
                cur = sc;
                if (sc > best) {
                    best = sc;
                    bestA = curA;
                    if (loop >= 1000) {
                        print_solution(C, bestA);
                    }
                }
            } else {
                curA[i][j1] += delta;
                curA[i][j2] -= delta;
                curA[j2][i] += delta;
                curA[j1][i] -= delta;
            }
        }
    }
    A = bestA;
    cerr << "loop = " << loop << "\n";
    long long best_score = compute_score(S, P, L, C, A);
    cerr << "Best Score = " << best_score << "\n";
}

// Simulated annealing focusing on the top four strings.
static void anneal_top4(const vector<string>& S, const vector<int>& P, long long L,
                        int M, vector<char>& bestC, vector<vector<int>>& bestA) {
    vector<int> ord(S.size());
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) { return P[a] > P[b]; });
    int use = min(4, (int)S.size());
    vector<string> T(use);
    vector<int> TP(use);
    for (int i = 0; i < use; i++) {
        T[i] = S[ord[i]];
        TP[i] = P[ord[i]];
    }

    string letters;
    for (int i = 0; i < use; i++) letters += T[i];
    if (letters.empty()) letters = "a";

    vector<char> C(M);
    for (int i = 0; i < M; i++) C[i] = letters[i % letters.size()];

    vector<vector<int>> adj(M);
    for (int i = 0; i < M; i++) {
        int deg = rand_int(2, 4);
        unordered_set<int> st;
        while ((int)st.size() < deg) {
            int v = rand_int(0, M - 1);
            if (v == i) continue;
            st.insert(v);
        }
        adj[i] = vector<int>(st.begin(), st.end());
    }

    adj_to_matrix(adj, bestA);
    bestC = C;
    long long cur = fast_score(T, TP, L, C, bestA);
    long long best = cur;

    auto start = chrono::steady_clock::now();
    
    while (chrono::duration<double>(chrono::steady_clock::now() - start).count() < TL) {
        auto cand = adj;
        int op = rand_int(0, 2);
        if (op == 0) {
            int u = rand_int(0, M - 1);
            int v = rand_int(0, M - 1);
            if (!cand[u].empty() && !cand[v].empty()) {
                int iu = rand_int(0, (int)cand[u].size() - 1);
                int iv = rand_int(0, (int)cand[v].size() - 1);
                int a = cand[u][iu];
                int b = cand[v][iv];
                if (find(cand[u].begin(), cand[u].end(), b) == cand[u].end() &&
                    find(cand[v].begin(), cand[v].end(), a) == cand[v].end()) {
                    cand[u][iu] = b;
                    cand[v][iv] = a;
                }
            }
        } else if (op == 1) {
            int u = rand_int(0, M - 1);
            if ((int)cand[u].size() < 4) {
                int v = rand_int(0, M - 1);
                if (v != u && find(cand[u].begin(), cand[u].end(), v) == cand[u].end()) {
                    cand[u].push_back(v);
                }
            }
        } else {
            int u = rand_int(0, M - 1);
            if ((int)cand[u].size() > 2) {
                int idx = rand_int(0, (int)cand[u].size() - 1);
                cand[u].erase(cand[u].begin() + idx);
            }
        }

        vector<vector<int>> Ak;
        adj_to_matrix(cand, Ak);
        long long sc = fast_score(T, TP, L, C, Ak);
        double t = START_TEMP * pow(END_TEMP / START_TEMP,
                                    chrono::duration<double>(chrono::steady_clock::now() - start).count() / TL);
        if (sc > cur || rand_double() < exp((double)(sc - cur) / t)) {
            adj.swap(cand);
            cur = sc;
            if (sc > best) {
                best = sc;
                bestA = Ak;
            }
        }
    }
    bestC = C;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    long long L;
    if (!(cin >> N >> M >> L)) return 0;
    vector<string> S(N);
    vector<int> P(N);
    for (int i = 0; i < N; i++) cin >> S[i] >> P[i];

    vector<int> ord(N);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) { return P[a] > P[b]; });
    int idx1 = ord[0];
    int idx2 = ord.size() >= 2 ? ord[1] : ord[0];

    vector<char> C(M);
    vector<vector<int>> A(M, vector<int>(M, 0));
    build_fixed_two(S[idx1], S[idx2], C, A);

    anneal_matrix(S, P, L, C, A);

    for (int i = 0; i < M; i++) {
        cout << C[i];
        for (int j = 0; j < M; j++) {
            cout << ' ' << A[i][j];
        }
        cout << "\n";
    }
    return 0;
}
