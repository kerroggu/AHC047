#include <bits/stdc++.h>
using namespace std;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N,M; long long L;
    if(!(cin>>N>>M>>L)) return 0;
    vector<string> S(N); vector<int> P(N);
    for(int i=0;i<N;i++) cin>>S[i]>>P[i];

    // count weighted bigram frequencies
    const int K=6; // letters a..f
    vector<vector<long long>> cnt(K, vector<long long>(K,0));
    for(int idx=0; idx<N; idx++){
        const string& w=S[idx];
        long long weight=P[idx];
        for(size_t i=0; i<w.size(); i++){
            char a=w[i];
            char b=w[(i+1)%w.size()];
            cnt[a-'a'][b-'a'] += weight;
        }
    }

    // pick top 30 edges
    struct Edge{int u,v; long long w;};
    vector<Edge> edges;
    for(int u=0; u<K; u++) for(int v=0; v<K; v++){
        edges.push_back({u,v,cnt[u][v]});
    }
    sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b){return a.w>b.w;});
    const int LIMIT=30;
    vector<vector<int>> allowed(K, vector<int>(K,0));
    for(int i=0; i<min((int)edges.size(), LIMIT); i++){
        allowed[edges[i].u][edges[i].v]=1;
    }

    // build state letters (12 states: abcdefabcdef)
    vector<char> C(M);
    string letters="abcdef";
    for(int i=0;i<M;i++) C[i]=letters[i%letters.size()];

    // gather state indices for each letter
    vector<vector<int>> by(K);
    for(int i=0;i<M;i++) by[C[i]-'a'].push_back(i);

    vector<vector<int>> A(M, vector<int>(M,0));
    for(int i=0;i<M;i++){
        int from=C[i]-'a';
        vector<int> dests;
        for(int v=0; v<K; v++) if(allowed[from][v]){
            for(int idx: by[v]) dests.push_back(idx);
        }
        if(dests.empty()){
            A[i][i]=100;
            continue;
        }
        int k=dests.size();
        for(int idx=0; idx<k; idx++) A[i][dests[idx]] = 100/k;
        for(int idx=0; idx<100%k; idx++) A[i][dests[idx]]++;
    }

    cerr << "Generated with " << LIMIT << " edges" << "\n";
    for(int i=0;i<M;i++){
        cout<<C[i];
        for(int j=0;j<M;j++) cout<<' '<<A[i][j];
        cout<<"\n";
    }
    return 0;
}

