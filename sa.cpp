#include <bits/stdc++.h>
using namespace std;

// Simulated annealing parameters
const double START_TEMP = 1e3;
const double END_TEMP = 1e-1;
const int MAX_ITER = 2000; // iteration count for annealing

struct Input {
    int N, M; long long L;
    vector<string> S; vector<int> P;
};

struct State {
    vector<char> C; // length M
    vector<vector<int>> A; // M x M probabilities (sum 100 each row)
};

// Compute prefix function for KMP
vector<int> prefix_function(const string& s){
    int n=s.size();
    vector<int> pi(n); pi[0]=0;
    for(int i=1;i<n;i++){
        int j=pi[i-1];
        while(j>0 && s[i]!=s[j]) j=pi[j-1];
        if(s[i]==s[j]) j++;
        pi[i]=j;
    }
    return pi;
}

// Build transition table for KMP automaton
vector<vector<int>> build_kmp_go(const string& pat, const vector<int>& pi){
    int L=pat.size();
    vector<vector<int>> go(L, vector<int>(6));
    for(int p=0;p<L;p++){
        for(int c=0;c<6;c++){
            int q=p;
            char ch='a'+c;
            while(q>0 && pat[q]!=ch) q=pi[q-1];
            if(pat[q]==ch) q++;
            if(q==L) go[p][c]=-1; // match completed
            else go[p][c]=q;
        }
    }
    return go;
}

// Multiply matrices (n x n)
vector<vector<double>> mul(const vector<vector<double>>& a,const vector<vector<double>>& b){
    int n=a.size();
    vector<vector<double>> c(n, vector<double>(n,0.0));
    for(int i=0;i<n;i++){
        for(int k=0;k<n;k++) if(a[i][k]!=0){
            for(int j=0;j<n;j++) c[i][j]+=a[i][k]*b[k][j];
        }
    }
    return c;
}

// Compute probability that word appears at least once
static double compute_word_prob(const string& word,long long L,const State& st){
    int M=st.C.size();
    int W=word.size();
    auto pi=prefix_function(word);
    auto go=build_kmp_go(word,pi);

    // enumerate states
    vector<vector<int>> idx(W, vector<int>(M,-1));
    int n=0;
    for(int j=0;j<M;j++){
        idx[0][j]=n++;
        for(int i=0;i<W-1;i++) if(word[i]==st.C[j]) idx[i+1][j]=n++;
    }
    vector<vector<double>> X(n, vector<double>(n,0.0));
    for(int j=0;j<M;j++){
        for(int len=0;len<W;len++) if(idx[len][j]!=-1){
            int from=idx[len][j];
            for(int v=0;v<M;v++){
                char ch=st.C[v];
                int nextlen=go[len][ch-'a'];
                if(nextlen==-1) continue; // matched
                int to=idx[nextlen][v];
                if(to!=-1) X[to][from]+=st.A[j][v]/100.0;
            }
        }
    }

    long long power=L-1;
    vector<vector<double>> Y(n, vector<double>(n,0.0));
    for(int i=0;i<n;i++) Y[i][i]=1.0;
    vector<vector<double>> Acur=X;
    while(power>0){
        if(power&1) Y=mul(Acur,Y);
        Acur=mul(Acur,Acur);
        power>>=1;
    }

    int init = (st.C[0]==word[0]) ? idx[1][0] : idx[0][0];
    double ret=1.0;
    for(int i=0;i<n;i++) ret-=Y[i][init];
    if(ret<0) ret=0; if(ret>1) ret=1;
    return ret;
}

static double compute_score(const Input& in,const State& st){
    double score=0.0;
    for(int i=0;i<in.N;i++){
        double q=compute_word_prob(in.S[i],in.L,st);
        score+=q*in.P[i];
    }
    return score;
}

// Generate initial state as described
State initial_state(const Input& in){
    int best=0;
    for(int i=1;i<in.N;i++) if(in.P[i]>in.P[best]) best=i;
    string T=in.S[best];
    int Ls=T.size();
    State st; st.C.resize(in.M); st.A.assign(in.M, vector<int>(in.M,0));
    for(int i=0;i<in.M;i++) st.C[i]=T[i%Ls];
    int base=59/(in.M-1); // integer division
    int rem=59 - base*(in.M-1);
    for(int i=0;i<in.M;i++){
        int next=(i+1)%in.M;
        for(int j=0;j<in.M;j++) st.A[i][j]=base;
        for(int j=0;j<rem;j++){
            int idx=(j==0?0:(next+j)%in.M);
            if(idx==next) idx=(idx+1)%in.M; // avoid next state
            st.A[i][idx]++;
        }
        st.A[i][next]=41;
        // adjust if we accidentally over-added to next state
        int sum=0; for(int j=0;j<in.M;j++) sum+=st.A[i][j];
        if(sum!=100){
            st.A[i][next]-=(sum-100);
        }
    }
    return st;
}

void mutate(State& st,mt19937& rng){
    uniform_int_distribution<int> dist_row(0,st.C.size()-1);
    int r=dist_row(rng);
    if(rng()%2==0){
        // modify letter
        char old=st.C[r];
        char newc='a'+rng()%6;
        st.C[r]=newc;
    }else{
        // move probability between two transitions
        int m=st.C.size();
        int a=rng()%m, b=rng()%m;
        if(a==b) b=(b+1)%m;
        if(st.A[r][a]>0){
            st.A[r][a]--; st.A[r][b]++;
        }
    }
}

int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    Input in; if(!(cin>>in.N>>in.M>>in.L)) return 0; in.S.resize(in.N); in.P.resize(in.N);
    for(int i=0;i<in.N;i++) cin>>in.S[i]>>in.P[i];
    mt19937 rng(0);
    State cur=initial_state(in); double cur_score=compute_score(in,cur);
    State best=cur; double best_score=cur_score;

    for(int iter=0;iter<MAX_ITER;iter++){
        double t = START_TEMP + (END_TEMP-START_TEMP)*(double)iter/MAX_ITER;
        State nxt=cur; mutate(nxt,rng);
        double nxt_score=compute_score(in,nxt);
        double diff=nxt_score-cur_score;
        if(diff>=0 || exp(diff/t)> (double)rng()/rng.max()){
            cur=nxt; cur_score=nxt_score;
            if(cur_score>best_score){ best=cur; best_score=cur_score; }
        }
    }

    for(int i=0;i<in.M;i++){
        cout<<best.C[i];
        for(int j=0;j<in.M;j++) cout<<' '<<best.A[i][j];
        cout<<"\n";
    }
    cerr<<"Best score="<<best_score<<"\n";
    return 0;
}
