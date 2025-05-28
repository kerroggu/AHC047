#include <bits/stdc++.h>
using namespace std;

// --- parameters ---
const double TL = 1.95;
const double START_TEMP = 1.0;
const double END_TEMP = 0.01;
const int EDGE_LIMIT = 30;

static std::mt19937 rng(123456789);
static inline double rand_double(){ return std::uniform_real_distribution<double>(0.0,1.0)(rng); }
static inline int rand_int(int l,int r){ return std::uniform_int_distribution<int>(l,r)(rng); }

// compute stationary distribution
static vector<double> compute_stationary(const vector<vector<int>>& A){
    int M=A.size();
    vector<double> pi(M,1.0/M), nxt(M);
    for(int it=0; it<200; it++){
        fill(nxt.begin(), nxt.end(), 0.0);
        for(int i=0;i<M;i++) for(int j=0;j<M;j++) nxt[j]+=pi[i]*A[i][j]*0.01;
        double diff=0.0;
        for(int i=0;i<M;i++) diff += fabs(nxt[i]-pi[i]);
        pi.swap(nxt);
        if(diff<1e-12) break;
    }
    return pi;
}

// probability to generate word once (approximate)
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

// product score (log)
static double product_score(const vector<string>& S, const vector<int>& P, long long L,
                            const vector<char>& C, const vector<vector<int>>& A){
    int M=C.size();
    for(int i=0;i<M;i++) if(accumulate(A[i].begin(),A[i].end(),0)!=100) return -1e100;
    vector<double> pi=compute_stationary(A);
    double sumLog=0.0;
    for(size_t idx=0; idx<S.size(); idx++){
        double p=approx_word_once(S[idx],C,A,pi);
        long long occ=L-(long long)S[idx].size()+1;
        double q=1e-12;
        if(occ>0){
            q=1.0-pow(1.0-p,occ);
            if(q<=0) q=1e-12;
        }
        sumLog += P[idx]*log(q);
    }
    return sumLog;
}

// build transition matrix from allowed edges
static void edges_to_matrix(const vector<char>& C,
                            const vector<vector<int>>& pos,
                            const array<array<int,6>,6>& used,
                            vector<vector<int>>& A){
    int M=C.size();
    A.assign(M, vector<int>(M,0));
    for(int i=0;i<M;i++){
        int from=C[i]-'a';
        vector<int> dests;
        for(int v=0; v<6; v++) if(used[from][v]){
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

    int N,M; long long L; if(!(cin>>N>>M>>L)) return 0;
    vector<string> S(N); vector<int> P(N);
    for(int i=0;i<N;i++) cin>>S[i]>>P[i];

    string letters="abcdefabcdef";
    vector<char> C(M);
    for(int i=0;i<M;i++) C[i]=letters[i%letters.size()];
    vector<vector<int>> pos(6);
    for(int i=0;i<M;i++) pos[C[i]-'a'].push_back(i);

    vector<pair<int,int>> allEdges;
    for(int u=0; u<6; u++) for(int v=0; v<6; v++) allEdges.push_back({u,v});

    // initial edges from top word
    int start_idx=max_element(P.begin(),P.end())-P.begin();
    array<array<int,6>,6> used={};
    int used_cnt=0;
    for(size_t i=0;i<S[start_idx].size();i++){
        int u=S[start_idx][i]-'a';
        int v=S[start_idx][(i+1)%S[start_idx].size()]-'a';
        if(!used[u][v]){ used[u][v]=1; used_cnt++; }
    }

    vector<vector<int>> bestA;
    edges_to_matrix(C,pos,used,bestA);
    double bestScore=product_score(S,P,L,C,bestA);

    // greedy addition
    while(used_cnt < EDGE_LIMIT){
        double curBest=bestScore; pair<int,int> bestEdge={-1,-1}; vector<vector<int>> curA;
        for(auto e: allEdges){
            int u=e.first,v=e.second; if(used[u][v]) continue;
            used[u][v]=1; used_cnt++;
            edges_to_matrix(C,pos,used,curA);
            double sc=product_score(S,P,L,C,curA);
            if(sc>curBest){ curBest=sc; bestEdge=e; }
            used[u][v]=0; used_cnt--;
        }
        if(bestEdge.first==-1) break;
        used[bestEdge.first][bestEdge.second]=1; used_cnt++;
        edges_to_matrix(C,pos,used,bestA); bestScore=product_score(S,P,L,C,bestA);
    }

    // simulated annealing
    array<array<int,6>,6> cur=used, best=used; int cur_cnt=used_cnt; double curScore=bestScore;
    auto start=chrono::steady_clock::now();
    while(chrono::duration<double>(chrono::steady_clock::now()-start).count() < TL){
        array<array<int,6>,6> cand=cur; int cand_cnt=cur_cnt;
        int op=rand_int(0,2);
        if(op==0){ // add
            if(cand_cnt>=EDGE_LIMIT) continue;
            vector<pair<int,int>> unused; for(auto e: allEdges) if(!cand[e.first][e.second]) unused.push_back(e);
            if(unused.empty()) continue;
            auto e=unused[rand_int(0,unused.size()-1)]; cand[e.first][e.second]=1; cand_cnt++;
        }else if(op==1){ // remove
            if(cand_cnt==0) continue;
            vector<pair<int,int>> usedv; for(auto e: allEdges) if(cand[e.first][e.second]) usedv.push_back(e);
            auto e=usedv[rand_int(0,usedv.size()-1)]; cand[e.first][e.second]=0; cand_cnt--;        
        }else{ // swap
            if(cand_cnt==0) continue;
            vector<pair<int,int>> usedv; vector<pair<int,int>> unused;
            for(auto e: allEdges){ if(cand[e.first][e.second]) usedv.push_back(e); else unused.push_back(e); }
            if(unused.empty()) continue; auto r1=usedv[rand_int(0,usedv.size()-1)]; auto r2=unused[rand_int(0,unused.size()-1)];
            cand[r1.first][r1.second]=0; cand[r2.first][r2.second]=1;
        }
        vector<vector<int>> A; edges_to_matrix(C,pos,cand,A);
        double sc=product_score(S,P,L,C,A);
        double t=START_TEMP*pow(END_TEMP/START_TEMP, chrono::duration<double>(chrono::steady_clock::now()-start).count()/TL);
        bool accept=false;
        if(sc>curScore) accept=true;
        else if(rand_double()<exp((sc-curScore)/t)) accept=true;
        if(accept){ cur=cand; cur_cnt=cand_cnt; curScore=sc; if(sc>bestScore){ bestScore=sc; best=cand; bestA.swap(A); } }
    }

    if(bestA.empty()) edges_to_matrix(C,pos,best,bestA);
    cerr<<"Log product score = "<<bestScore<<"\n";
    for(int i=0;i<M;i++){
        cout<<C[i];
        for(int j=0;j<M;j++) cout<<' '<<bestA[i][j];
        cout<<'\n';
    }
    return 0;
}

