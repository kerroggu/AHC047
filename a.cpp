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

    vector<char> C(M);
    vector<vector<int>> A(M, vector<int>(M, 0));

    build(S[idx1], 0, 6, C, A);  // states 0..5
    build(S[idx2], 6, 0, C, A);  // states 6..11

    for (int i = 0; i < M; i++) {
        cout << C[i];
        for (int j = 0; j < M; j++) {
            cout << ' ' << A[i][j];
        }
        cout << "\n";
    }
    return 0;
}
