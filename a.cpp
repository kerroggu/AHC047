#include <bits/stdc++.h>
using namespace std;

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

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    long long L;
    if (!(cin >> N >> M >> L)) return 0;
    vector<string> S(N);
    vector<int> P(N);
    for (int i = 0; i < N; i++) cin >> S[i] >> P[i];

    // Find indices of the top two scoring strings.
    vector<int> ord(N);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) { return P[a] > P[b]; });
    int idx1 = ord[0];
    int idx2 = ord.size() >= 2 ? ord[1] : ord[0];

    vector<char> C1(M); // top two strings
    vector<vector<int>> A1(M, vector<int>(M, 0));
    build(S[idx1], 0, 6, C1, A1);
    build(S[idx2], 6, 0, C1, A1);
    long long score1 = compute_score(S, P, L, C1, A1);

    vector<char> C2(M); // top one string only
    vector<vector<int>> A2(M, vector<int>(M, 0));
    build_single(S[idx1], C2, A2);
    long long score2 = compute_score(S, P, L, C2, A2);

    vector<char> bestC = C1;
    vector<vector<int>> bestA = A1;
    long long bestScore = score1;
    if (score2 > bestScore) {
        bestScore = score2;
        bestC = C2;
        bestA = A2;
    }

    // Additional patterns using top k strings (k = 2,3,4,5)
    for (int k = 2; k <= 5; k++) {
        if (k > N) break;
        vector<char> Ck(M);
        vector<vector<int>> Ak(M, vector<int>(M, 0));
        build_multi_k(S, ord, k, Ck, Ak);
        long long sc = compute_score(S, P, L, Ck, Ak);
        if (sc > bestScore) {
            bestScore = sc;
            bestC = Ck;
            bestA = Ak;
        }
    }

    // Sequential allocation variant of the above approach
    for (int k = 2; k <= 5; k++) {
        if (k > N) break;
        vector<char> Ck(M);
        vector<vector<int>> Ak(M, vector<int>(M, 0));
        build_multi_k_seq(S, ord, k, Ck, Ak);
        long long sc = compute_score(S, P, L, Ck, Ak);
        if (sc > bestScore) {
            bestScore = sc;
            bestC = Ck;
            bestA = Ak;
        }
    }

    for (int i = 0; i < M; i++) {
        cout << bestC[i];
        for (int j = 0; j < M; j++) {
            cout << ' ' << bestA[i][j];
        }
        cout << "\n";
    }
    return 0;
}
