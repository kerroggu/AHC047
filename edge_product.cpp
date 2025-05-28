#include <bits/stdc++.h>
using namespace std;

static const int EDGE_LIMIT = 30;

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

// Approximate probability to generate word once
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

static double product_score(const vector<string>& S, const vector<int>& P, long long L,
                            const vector<char>& C, const vector<vector<int>>& A){
    int M=C.size();
    for(int i=0;i<M;i++) if(accumulate(A[i].begin(),A[i].end(),0)!=100) return -1e100;
    vector<double> pi=compute_stationary(A);
    double sumLog=0.0;
    for(size_t idx=0; idx<S.size(); idx++){
        double p=approx_word_once(S[idx],C,A,pi);
        long long occ=L-(long long)S[idx].size()+1;
        double q=0.0;
        if(occ>0){
            q=1.0-pow(1.0-p, occ);
            if(q<=0) q=1e-12;
        }else q=1e-12;
        sumLog += P[idx]*log(q);
    }
    return sumLog; // log of product^weights
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N,M; long long L; if(!(cin>>N>>M>>L)) return 0;
    vector<string> S(N); vector<int> P(N);
    for(int i=0;i<N;i++) cin>>S[i]>>P[i];

    string letters="abcdefabcdef";
    vector<char> C(M);
    for(int i=0;i<M;i++) C[i]=letters[i%letters.size()];
    vector<vector<int>> pos(6);
    for(int i=0;i<M;i++) pos[C[i]-'a'].push_back(i);

    // all edges
    vector<pair<int,int>> allEdges;
    for(int u=0; u<6; u++) for(int v=0; v<6; v++) allEdges.push_back({u,v});

    // start with edges from highest weight word
    int start_idx = max_element(P.begin(), P.end())-P.begin();
    set<pair<int,int>> used;
    for(size_t i=0;i<S[start_idx].size();i++){
        int u=S[start_idx][i]-'a';
        int v=S[start_idx][(i+1)%S[start_idx].size()]-'a';
        used.insert({u,v});
    }
    vector<vector<int>> bestA;
    edges_to_matrix(C,pos,used,bestA);
    double bestScore=product_score(S,P,L,C,bestA);

    for(int step=used.size(); step<EDGE_LIMIT; step++){
        double curBest=bestScore;
        pair<int,int> bestEdge={-1,-1};
        vector<vector<int>> curA;
        for(auto e: allEdges){
            if(used.count(e)) continue;
            set<pair<int,int>> cand=used; cand.insert(e);
            vector<vector<int>> A;
            edges_to_matrix(C,pos,cand,A);
            double sc=product_score(S,P,L,C,A);
            if(sc>curBest){
                curBest=sc; bestEdge=e; curA.swap(A);
            }
        }
        if(bestEdge.first==-1) break;
        used.insert(bestEdge); bestScore=curBest; bestA.swap(curA);
    }

    cerr<<"Log product score = "<<bestScore<<"\n";
    for(int i=0;i<M;i++){
        cout<<C[i];
        for(int j=0;j<M;j++) cout<<' '<<bestA[i][j];
        cout<<'\n';
    }
    return 0;
}

