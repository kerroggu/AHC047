#include <bits/stdc++.h>
using namespace std;
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int N,M; long long L;
    if(!(cin>>N>>M>>L)) return 0;
    vector<string>S(N); vector<int>P(N);
    for(int i=0;i<N;i++) cin>>S[i]>>P[i];
    int best=0;
    for(int i=1;i<N;i++) if(P[i]>P[best]) best=i;
    string T=S[best];
    int Ls=T.size();
    for(int i=0;i<M;i++){
        char c=T[i%Ls];
        int next=(i+1)%Ls; // deterministic cycle over best string
        cout<<c;
        for(int j=0;j<M;j++){
            cout<<' '<<(j==next?100:0);
        }
        cout<<"\n";
    }
    return 0;
}
