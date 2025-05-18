#include <bits/stdc++.h>
using namespace std;


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    long long L;
    if (!(cin >> N >> M >> L)) return 0;
    vector<string> S(N);
    vector<int> P(N);
    for (int i = 0; i < N; i++) cin >> S[i] >> P[i];

    // sort indices by score
    vector<int> ord(N);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) { return P[a] > P[b]; });

    int K = min(4, N); // use up to top 4 strings

    // Count transitions for each letter using the top strings
    vector<vector<int>> cnt(6, vector<int>(6, 0));
    for (int t = 0; t < K; t++) {
        const string& s = S[ord[t]];
        int len = s.size();
        for (int i = 0; i < len; i++) {
            int a = s[i] - 'a';
            int b = s[(i + 1) % len] - 'a';
            cnt[a][b]++;
        }
    }

    // Determine up to 3 next letters for each letter
    vector<vector<int>> nxt(6);
    for (int a = 0; a < 6; a++) {
        vector<pair<int,int>> tmp;
        for (int b = 0; b < 6; b++) tmp.push_back({-cnt[a][b], b});
        sort(tmp.begin(), tmp.end());
        for (int j = 0; j < 3 && j < 6; j++) {
            if (cnt[a][tmp[j].second] == 0 && j > 0) break;
            nxt[a].push_back(tmp[j].second);
        }
        if (nxt[a].empty()) nxt[a].push_back(a); // fallback to itself
    }

    const int STATE_PER_CHAR = 2; // total states = 6 * 2 = 12
    vector<char> C(M);
    vector<vector<int>> A(M, vector<int>(M, 0));
    for (int c = 0; c < 6; c++) {
        for (int k = 0; k < STATE_PER_CHAR; k++) {
            C[c * STATE_PER_CHAR + k] = 'a' + c;
        }
    }

    int start_state = (S[ord[0]][0] - 'a') * STATE_PER_CHAR;

    for (int c = 0; c < 6; c++) {
        for (int k = 0; k < STATE_PER_CHAR; k++) {
            int idx = c * STATE_PER_CHAR + k;
            vector<int> row(M, 0);
            row[start_state] = 1; // 1% jump to start

            int num = nxt[c].size();
            int prob = (num == 3 ? 33 : 41);
            int remaining = 99 - prob * num;

            for (int d : nxt[c]) {
                row[d * STATE_PER_CHAR] += prob;
            }

            // distribute remaining between the two states of this letter
            row[c * STATE_PER_CHAR] += remaining / 2;
            row[c * STATE_PER_CHAR + 1] += remaining - remaining / 2;

            A[idx] = row;
        }
    }

    for (int i = 0; i < M; i++) {
        cout << C[i];
        for (int j = 0; j < M; j++) {
            cout << ' ' << A[i][j];
        }
        cout << "\n";
    }
    return 0;
}
