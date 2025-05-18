#include <bits/stdc++.h>
using namespace std;
const double T0=50,T1=0.1;
const double TLE = 1.99+20;
constexpr int N = 20;
constexpr int M = 40;
constexpr long long NEG_INF = -1'000'000'000'000LL;
const int cnt_for_nei = 5;

struct Dir { int dx, dy; char ch; };
const array<Dir,4> DIRS = { Dir{-1,0,'U'}, Dir{1,0,'D'},
                            Dir{0,-1,'L'}, Dir{0,1,'R'} };

inline int cid(int x,int y){ return x*N + y; }
inline pair<int,int> xy(int id){ return {id/N, id%N}; }

/* ---------- one-step neighbours ---------- */
array<array<int,4>,N*N> STEP_NEI;
void initStep(){
    for(int x=0;x<N;++x)for(int y=0;y<N;++y){
        int id=cid(x,y);
        for(int d=0; d<4; ++d){
            int nx=x+DIRS[d].dx, ny=y+DIRS[d].dy;
            STEP_NEI[id][d] = (nx<0||nx>=N||ny<0||ny>=N)? -1 : cid(nx,ny);
        }
    }
}

/* ---------- BFS (8-move) ---------- */
vector<pair<char,char>>
_bfsPath(int src,int dst,const array<uint8_t,N*N>& blocked){
    if(src==dst) return {};
    static array<short,N*N> prev; prev.fill(-1);
    static array<uint8_t,N*N> act;  act.fill(0);   // 0=M 1=S
    static array<uint8_t,N*N> dirc; dirc.fill(0);  // 0..3

    deque<int> q; q.push_back(src); prev[src]=src;
    
    while(!q.empty()){
        int v=q.front(); q.pop_front();
        auto [vx,vy]=xy(v);

        /* moves */
        for(int d=0; d<4; ++d){
            int u=STEP_NEI[v][d];
            if(u<0||blocked[u]||prev[u]!=-1) continue;
            prev[u]=v; act[u]=0; dirc[u]=d;
            if(u==dst){ q.clear(); break; }
            q.push_back(u);
        }
        /* slides */
        for(int d=0; d<4; ++d){
            auto [dx,dy,_]=DIRS[d];
            int nx=vx, ny=vy;
            while(true){
                nx+=dx; ny+=dy;
                if(nx<0||nx>=N||ny<0||ny>=N) break;
                int uid=cid(nx,ny);
                if(blocked[uid]) break;
            }
            nx-=DIRS[d].dx; ny-=DIRS[d].dy;
            int stop=cid(nx,ny);
            if(stop==v||blocked[stop]||prev[stop]!=-1) continue;
            prev[stop]=v; act[stop]=1; dirc[stop]=d;
            if(stop==dst){ q.clear(); break; }
            q.push_back(stop);
        }
    }
    if(prev[dst]==-1) return {};

    vector<pair<char,char>> path;
    for(int cur=dst; cur!=src; cur=prev[cur])
        path.emplace_back(act[cur]?'S':'M', DIRS[dirc[cur]].ch);
    reverse(path.begin(), path.end());
    return path;
}

// ...existing code...

/* ---------- BFS (8-move) ---------- */
// visited 配列用のバージョン番号
static unsigned int bfs_visit_version = 0;
// 各ノードが最後に訪問されたバージョンを記録
static array<unsigned int, N*N> visited_version;
// 経路復元用の predecessor 配列 (初期化不要)
static array<short, N*N> bfs_prev;
// アクションタイプ (初期化不要)
static array<uint8_t, N*N> bfs_act;
// 方向 (初期化不要)
static array<uint8_t, N*N> bfs_dirc;

vector<pair<char,char>>
bfsPath(int src,int dst,const array<uint8_t,N*N>& blocked){
    if(src==dst) return {};

    // バージョン番号をインクリメント (0 は未訪問を示すためにスキップ)
    if (++bfs_visit_version == 0) {
        // オーバーフローした場合、visited_version をクリアする必要があるかもしれないが、
        // unsigned int の最大値まで到達することは稀なので、ここでは単純に 1 に設定
        bfs_visit_version = 1;
        // 必要であればここで visited_version.fill(0); を呼ぶ
    }

    deque<int> q;
    q.push_back(src);
    visited_version[src] = bfs_visit_version; // 現在のバージョンで訪問済みとする
    bfs_prev[src] = src; // 始点の predecessor は自分自身

    bool found = false; // dst が見つかったかどうかのフラグ

    while(!q.empty()){
        int v=q.front(); q.pop_front();
        auto [vx,vy]=xy(v);

        /* moves */
        for(int d=0; d<4; ++d){
            int u=STEP_NEI[v][d];
            // visited_version[u] != bfs_visit_version で未訪問かチェック
            if(u<0||blocked[u]||visited_version[u] == bfs_visit_version) continue;
            visited_version[u] = bfs_visit_version; // 訪問済みにする
            bfs_prev[u]=v; bfs_act[u]=0; bfs_dirc[u]=d;
            if(u==dst){ found = true; q.clear(); break; } // 見つかったらループを抜ける
            q.push_back(u);
        }
        if (found) break; // moves で見つかった場合

        /* slides */
        for(int d=0; d<4; ++d){
            auto [dx,dy,_]=DIRS[d];
            int nx=vx, ny=vy;
            while(true){
                nx+=dx; ny+=dy;
                if(nx<0||nx>=N||ny<0||ny>=N) break;
                int uid=cid(nx,ny);
                if(blocked[uid]) break;
            }
            nx-=DIRS[d].dx; ny-=DIRS[d].dy;
            int stop=cid(nx,ny);
            // visited_version[stop] != bfs_visit_version で未訪問かチェック
            if(stop==v||blocked[stop]||visited_version[stop] == bfs_visit_version) continue;
            visited_version[stop] = bfs_visit_version; // 訪問済みにする
            bfs_prev[stop]=v; bfs_act[stop]=1; bfs_dirc[stop]=d;
            if(stop==dst){ found = true; q.clear(); break; } // 見つかったらループを抜ける
            q.push_back(stop);
        }
         if (found) break; // slides で見つかった場合
    }

    // visited_version[dst] != bfs_visit_version で dst に到達できなかったかチェック
    if(visited_version[dst] != bfs_visit_version) return {};

    vector<pair<char,char>> path;
    // 経路復元 (bfs_prev, bfs_act, bfs_dirc を使用)
    for(int cur=dst; cur!=src; cur=bfs_prev[cur])
        path.emplace_back(bfs_act[cur]?'S':'M', DIRS[bfs_dirc[cur]].ch);
    reverse(path.begin(), path.end());
    return path;
}

/* ---------- route ---------- */
struct Route{ bool ok; vector<pair<char,char>> mv; };

Route buildRoute(const unordered_set<int>& blk,
                 const vector<int>& tgt, int start){
    array<uint8_t,N*N> placed{};          // already placed blocks
    vector<pair<char,char>> actions;
    int cur=start;

    auto place = [&](int id){
        auto [x,y]=xy(id);
        for(int d=0; d<4; ++d){
            int nx=x+DIRS[d].dx, ny=y+DIRS[d].dy;
            if(nx<0||nx>=N||ny<0||ny>=N) continue;
            int nid=cid(nx,ny);
            if(blk.count(nid)&&!placed[nid]){
                actions.emplace_back('A',DIRS[d].ch);
                placed[nid]=1;
            }
        }
    };

    place(cur);
    bool ok=true;

    for(int dst: tgt){
        place(cur);                           // once per segment
        auto seg=bfsPath(cur,dst,placed);
        if(seg.empty()){ ok=false; break; }

        for(auto [act,dch]:seg){
            actions.emplace_back(act,dch);
            int dIdx=string("UDLR").find(dch);
            int dx=DIRS[dIdx].dx, dy=DIRS[dIdx].dy;
            if(act=='M'){
                cur=STEP_NEI[cur][dIdx];
            }else{
                auto [x,y]=xy(cur);
                while(true){
                    int nx=x+dx, ny=y+dy;
                    if(nx<0||nx>=N||ny<0||ny>=N) break;
                    int nid=cid(nx,ny);
                    if(placed[nid]) break;
                    x=nx; y=ny;
                }
                cur=cid(x,y);
            }
        }
        place(cur);                           // at target
    }
    return {ok, move(actions)};
}

/* ---------- scoring ---------- */
inline long long score(bool ok,size_t len){
    return ok? (M-1*0)+2*N*(M-1*0)-static_cast<long long>(len)
             : NEG_INF+static_cast<long long>(len);
}

/* ---------- SA ---------- */
vector<pair<char,char>>
anneal(const vector<int>& tgt,int start,const vector<int>& cand,double lim=TLE){
    mt19937 rng(1234567);
    unordered_set<int> curS,bestS;
    auto curR=buildRoute(curS,tgt,start);
    long long curSc=score(curR.ok,curR.mv.size()),
              bestSc=curSc;
    auto bestMv=curR.mv;

    auto t0=chrono::steady_clock::now();
    
    uniform_real_distribution<> urd(0,1);
    int iter=0,lp=0;
    double t=0.0;
    while(true){
        iter++;
        lp++;
        if (lp==20){
            lp=0;
            //cerr<<iter<<' '<<curSc<<' '<<bestSc<<'\n';
            t=chrono::duration<double>(chrono::steady_clock::now()-t0).count()/lim;
        }
        
        if(t>=1.0) break;
        double temp=T0*pow(T1/T0,t);
        //double temp=T0 +(1.0-t)*(T1-T0);

        unordered_set<int> nxt=curS;
        double p=urd(rng);
        if(!curS.empty()&&p<0.4){
            int k=uniform_int_distribution<>(0,(int)curS.size()-1)(rng);
            auto it=curS.begin(); advance(it,k); nxt.erase(*it);
            int c=cand[uniform_int_distribution<>(0,(int)cand.size()-1)(rng)];
            nxt.insert(c);
        }else if(!curS.empty()&&p<0.7){
            int k=uniform_int_distribution<>(0,(int)curS.size()-1)(rng);
            auto it=curS.begin(); advance(it,k); nxt.erase(*it);
        }else{
            int c=cand[uniform_int_distribution<>(0,(int)cand.size()-1)(rng)];
            nxt.insert(c);
        }
        auto nxtR=buildRoute(nxt,tgt,start);
        long long nxtSc=score(nxtR.ok,nxtR.mv.size());

        bool ac=(nxtSc>curSc)||
                (exp(double(nxtSc-curSc)/temp)>urd(rng));
        if(ac){
            curS.swap(nxt); curSc=nxtSc;
            if(curSc>bestSc){ bestSc=curSc; bestMv.swap(nxtR.mv); }
        }
    }
    cerr<<"BestScore="<<bestSc<<'\n';
    cerr<<"Iter="<<iter<<'\n';
    return bestMv;
}

/* ---------- main ---------- */
// ...existing code...

/* ---------- main ---------- */
int main(){
    
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    initStep();

    int Nin,Min; if(!(cin>>Nin>>Min)) return 0;   // 20 40
    vector<pair<int,int>> pts(M);
    for(auto& p:pts) cin>>p.first>>p.second;

    int start=cid(pts[0].first,pts[0].second);
    vector<int> tgt; for(int i=1;i<M;++i)
        tgt.push_back(cid(pts[i].first,pts[i].second));

    /* candidate cells */
    vector<uint8_t> used(N*N,0);
    used[start]=1; for(int id:tgt) used[id]=1;
    vector<int> cand;
    for(int id=0;id<N*N;++id){
        if(used[id]) continue; // スタート地点と目的地は候補から除外

        auto [x,y] = xy(id);
        bool is_border = (x == 0 || x == N - 1 || y == 0 || y == N - 1);
        bool is_adjacent_to_used = false;

        for(int d=0; d<4; ++d){
            int nid = STEP_NEI[id][d];
            if(nid != -1 && used[nid]){
                is_adjacent_to_used = true;
                break;
            }
        }

        if(is_border || is_adjacent_to_used){
            for(int i = 0; i < cnt_for_nei - 1; i++) cand.push_back(id);
        }
        cand.push_back(id);
    }


    auto mv=anneal(tgt,start,cand);

    for(auto [a,d]:mv) cout<<a<<' '<<d<<'\n';
    return 0;
}