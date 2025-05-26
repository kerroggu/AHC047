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

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N,M; long long L;
    if(!(cin>>N>>M>>L)) return 0;
    vector<string> S(N); vector<int> P(N);
    for(int i=0;i<N;i++) cin>>S[i]>>P[i];

    vector<int> ord(N); iota(ord.begin(),ord.end(),0);
    sort(ord.begin(),ord.end(),[&](int a,int b){ return P[a]>P[b]; });
    string s1=S[ord[0]];
    string s2=S[ord.size()>=2?ord[1]:ord[0]];

    const string letters="abcdefabcdef";
    vector<char> C(M);
    for(int i=0;i<M;i++) C[i]=letters[i%letters.size()];

    vector<vector<int>> pos(6);
    for(int i=0;i<M;i++) pos[C[i]-'a'].push_back(i);

    auto next_char=[&](const string& s,char c){
        for(size_t i=0;i<s.size();i++) if(s[i]==c) return s[(i+1)%s.size()];
        return c;
    };

    vector<int> dest1(M), dest2(M);
    for(int i=0;i<M;i++){
        char c=C[i];
        char n1=next_char(s1,c);
        char n2=next_char(s2,c);
        dest1[i]=pos[n1-'a'][0];
        if(pos[n2-'a'].size()>1) dest2[i]=pos[n2-'a'][1];
        else dest2[i]=pos[n2-'a'][0];
    }

    vector<vector<int>> bestA;
    long long bestScore=-1;
    for(int mask=0; mask<(1<<M); mask++){
        vector<vector<int>> adj(M);
        for(int i=0;i<M;i++){
            int d=(mask>>i)&1 ? dest2[i] : dest1[i];
            adj[i].push_back(d);
        }
        vector<vector<int>> A;
        adj_to_matrix(adj,A);
        long long sc=fast_score(S,P,L,C,A);
        if(sc>bestScore){
            bestScore=sc;
            bestA=A;
        }
    }

    cerr<<"Best Score = "<<bestScore<<"\n";
    for(int i=0;i<M;i++){
        cout<<C[i];
        for(int j=0;j<M;j++) cout<<' '<<bestA[i][j];
        cout<<'\n';
    }
    return 0;
}

