#include <bits/stdc++.h>
using namespace std;

const double TL = 1.95;
static std::mt19937 rng(123456789);
static inline double rand_double(){ return std::uniform_real_distribution<double>(0.0,1.0)(rng); }
static inline int rand_int(int l,int r){ return std::uniform_int_distribution<int>(l,r)(rng); }
double qpow=0.4;

// Compute stationary distribution of transition matrix
static vector<double> compute_stationary(const vector<vector<int>>& A){
    int M=A.size();
    vector<double> pi(M,1.0/M), nxt(M);
    for(int it=0; it<200; it++){
        fill(nxt.begin(), nxt.end(), 0.0);
        for(int i=0;i<M;i++) for(int j=0;j<M;j++) nxt[j]+=pi[i]*A[i][j]*0.01;
        double diff=0.0;
        for(int i=0;i<M;i++) diff += fabs(nxt[i]-pi[i]);
        pi.swap(nxt);
        if(diff < 1e-12) break;
    }
    return pi;
}

static double approx_word_once(const string& word, const vector<char>& C,
                               const vector<vector<int>>& A,
                               const vector<double>& pi){
    int M=C.size();
    vector<vector<int>> by(6);
    for(int i=0;i<M;i++) by[C[i]-'a'].push_back(i);
    vector<double> dp(M,0.0);
    for(int s: by[word[0]-'a']) dp[s]+=pi[s];
    for(size_t t=1;t<word.size();t++){
        vector<double> nx(M,0.0);
        for(int i=0;i<M;i++) if(dp[i]>0.0) for(int j: by[word[t]-'a']) nx[j]+=dp[i]*A[i][j]*0.01;
        dp.swap(nx);
    }
    double p=0.0; for(double v: dp) p+=v; return p;
}

static long long fast_score(const vector<string>& S, const vector<int>& P, long long L,
                            const vector<char>& C, const vector<vector<int>>& A){
    int M=C.size();
    for(int i=0;i<M;i++) if(accumulate(A[i].begin(),A[i].end(),0)!=100) return 0;
    vector<double> pi=compute_stationary(A);
    double total=0.0;
    for(size_t i=0;i<S.size();i++){
        double p=approx_word_once(S[i],C,A,pi);
        long long occ=L-(long long)S[i].size()+1;
        if(occ>0){
            double q=1.0-pow(1.0-p,occ);
            q=max(0.0,min(1.0,q));
            q=pow(q,qpow);
            total+=q*P[i];
        }
    }
    return llround(total);
}

static void adj_to_matrix(const vector<vector<int>>& adj, vector<vector<int>>& A){
    int M=adj.size();
    A.assign(M, vector<int>(M,0));
    for(int i=0;i<M;i++){
        int deg=adj[i].size();
        if(deg==0){ A[i][i]=100; continue; }
        int per;
        if(deg==2) per=41;
        else if(deg==3) per=32;
        else if(deg==4) per=24;
        else per=100/deg;
        int sum=0; for(int to: adj[i]){ A[i][to]+=per; sum+=per; }
        int rem=100-sum;
        for(int j=0;j<M;j++) A[i][j]+=rem/M;
        for(int j=0;j<rem%M;j++) A[i][j]++;
    }
}

static void edges_to_matrix(const vector<char>& C,
                            const vector<vector<int>>& pos,
                            const set<pair<int,int>>& edges,
                            vector<vector<int>>& A){
    int M=C.size();
    A.assign(M, vector<int>(M,0));
    for(int i=0;i<M;i++){
        int from=C[i]-'a';
        vector<int> dests;
        for(int v=0; v<6; v++) if(edges.count({from,v})){
            for(int to: pos[v]) dests.push_back(to);
        }
        if(dests.empty()){
            A[i][i]=100;
        }else{
            int k=dests.size();
            for(int idx=0; idx<k; idx++) A[i][dests[idx]]=100/k;
            for(int idx=0; idx<100%k; idx++) A[i][dests[idx]]++;
        }
    }
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N,M; long long L;
    if(!(cin>>N>>M>>L)) return 0;
    vector<string> S(N); vector<int> P(N);
    for(int i=0;i<N;i++) cin>>S[i]>>P[i];

    const string letters="abcdefabcdef";
    vector<char> C(M);
    for(int i=0;i<M;i++) C[i]=letters[i%letters.size()];

    vector<vector<int>> pos(6);
    for(int i=0;i<M;i++) pos[C[i]-'a'].push_back(i);

    vector<pair<int,int>> allEdges;
    for(int u=0;u<6;u++) for(int v=0;v<6;v++) allEdges.push_back({u,v});

    // start with edges from the best scoring word
    int best_idx=max_element(P.begin(), P.end())-P.begin();
    const string& base=S[best_idx];
    set<pair<int,int>> used;
    for(size_t i=0;i<base.size();i++){
        int u=base[i]-'a';
        int v=base[(i+1)%base.size()]-'a';
        used.insert({u,v});
    }
    vector<vector<int>> bestA;
    long long bestScore=-1;

    // initial score with no edges (all self loops)
    edges_to_matrix(C,pos,used,bestA);
    bestScore=fast_score(S,P,L,C,bestA);

    for(int step=0; step<30; step++){
        long long curBest=bestScore;
        vector<vector<int>> curA=bestA;
        pair<int,int> bestEdge={-1,-1};
        for(auto e: allEdges){
            if(used.count(e)) continue;
            set<pair<int,int>> cand=used;
            cand.insert(e);
            vector<vector<int>> A;
            edges_to_matrix(C,pos,cand,A);
            long long sc=fast_score(S,P,L,C,A);
            if(sc>curBest){
                curBest=sc;
                curA.swap(A);
                bestEdge=e;
            }
        }
        if(bestEdge.first==-1) break;
        used.insert(bestEdge);
        bestScore=curBest;
        bestA=curA;
    }

    cerr<<"Best Score = "<<bestScore<<"\n";
    for(int i=0;i<M;i++){
        cout<<C[i];
        for(int j=0;j<M;j++) cout<<' '<<bestA[i][j];
        cout<<'\n';
    }
    return 0;
}

