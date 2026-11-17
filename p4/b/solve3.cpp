// p4-b: Masyu by edge constraint propagation + DFS, then randomised restarts for shorter loops.
// Edge states: -1 unknown, 0 off, 1 on. Rules: cell degree 0 or 2 (pearls 2), black/white pearl
// rules, and no sub-loop: chains keep partner endpoints; closing a chain is only allowed when it
// holds every on-edge and every pearl. Branch at the chain end with the fewest unknown edges.
#include <bits/stdc++.h>
using namespace std;

namespace P4CFG {
double FIRST_DEADLINE_S = 5.5;   // keep searching for a first valid loop up to this time
double IMPROVE_UNTIL_S = 0.8;    // after a loop is found, try restarts for shorter loops until here
long long RESTART_NODES = 20000; // node limit of one randomised restart (improvement phase)
double HARD_STOP_S = 5.5;        // DFS aborts at this time whatever happens
double ORDER_NOISE = 3.0;        // random noise (cells) added to the distance ordering on restarts
int PROBE_ROOT = 0;              // 1 = failed-literal probing on all edges at the root
int PROBE_NODE = 0;              // 1 = probe the branch cell's unknown edges at every node
int PROBE_ROUNDS = 4;            // max probing sweeps
int CONN = 1;                    // 1 = prune when pearls / loop cells are disconnected
int ALT_FOCUS = 0;               // 1 = alternate FOCUS between restarts while no loop is found
int FOCUS = 0;                   // branch on the longest chain's end (1) or the most constrained end (0)
long long FIRST_NODES = 2000;    // node limit of the first (deterministic) search, doubled per restart
int ORDER = 0;                   // value order: 0 = towards target, 1 = Warnsdorff
double NODE_GROWTH = 1.0;        // restart node limit multiplier while no loop is found
long long PICK_WORK = 200000;    // FOCUS 2: max end x target pairs scanned when picking a new focus
int PEARL_FIRST = 0;             // 1 = decide every pearl's edges before extending chains
double SA_T0 = 2.0;              // loop-shape SA start temperature (violation units)
double SA_T1 = 0.05;             // loop-shape SA end temperature
double SA_LAMBDA = 0.003;        // loop-shape SA cost per loop cell (prefers short loops)
int BB = 1;                      // 1 = prune restarts with on-edges >= best length (branch and bound)
}

// ---- geometric rectangle / notch loops, copied from p4-a's baseline (best/p4/BEST.cpp) ----
namespace P4_CONFIG {
constexpr double TIME_BUDGET_S=8.0;
constexpr int CLOCK_CHECK_NODES=64;
constexpr double GEOMETRY_BUDGET_S=0.3;  // p4-b: cap the shape enumeration
}
namespace p4builder {
const int masks[7]={0,3,5,6,9,10,12};
const char symbols[7]={'.','L','|','F','J','-','7'};
const int dr[4]={-1,0,1,0},dc[4]={0,1,0,-1};
struct Solver {
 int R,C,N; vector<string> input; vector<array<int,4>> adj;
 chrono::steady_clock::time_point start; long long nodes=0; bool expired=false;
 vector<unsigned char> answer;
 bool straight(int m){return m==5||m==10;}
 bool timed(){return chrono::duration<double>(chrono::steady_clock::now()-start).count()>P4_CONFIG::TIME_BUDGET_S;}
 bool propagate(vector<unsigned char>& dom){
  bool change=true;
  while(change){
   change=false;
   if(timed()){expired=true;return false;}
   for(int v=0;v<N;v++){
    int keep=0;
    for(int k=0;k<7;k++)if(dom[v]>>k&1){
     int m=masks[k];bool good=true,whiteTurn=false;
     for(int d=0;d<4&&good;d++){
      bool edge=m>>d&1;int u=adj[v][d];
      if(u<0){if(edge)good=false;continue;}
      bool support=false;
      for(int l=0;l<7;l++)if(dom[u]>>l&1){
       int q=masks[l];if(bool(q>>((d+2)%4)&1)!=edge)continue;
       if(edge && input[v/C][v%C]=='B' && !straight(q))continue;
       if(edge && input[u/C][u%C]=='B' && !straight(m))continue;
       support=true;
       if(edge&&q&&!straight(q))whiteTurn=true;
      }
      if(!support)good=false;
     }
     if(input[v/C][v%C]=='W'&&!whiteTurn)good=false;
     if(good)keep|=1<<k;
    }
    if(!keep)return false;
    if(keep!=dom[v]){dom[v]=keep;change=true;}
   }
   // A closed component cannot be extended. It must contain every pearl.
   vector<char> seen(N);
   for(int s=0;s<N;s++)if(!seen[s]&&__builtin_popcount((unsigned)dom[s])==1&&dom[s]!=1){
    vector<int> comp={s};seen[s]=1;bool closed=true;
    for(int z=0;z<(int)comp.size();z++){
     int v=comp[z],m=masks[__builtin_ctz((unsigned)dom[v])];
     for(int d=0;d<4;d++)if(m>>d&1){int u=adj[v][d];
      if(u<0)return false;
      if(__builtin_popcount((unsigned)dom[u])!=1){closed=false;continue;}
      if(!seen[u]){seen[u]=1;comp.push_back(u);}
     }
    }
    if(closed){
     vector<char> in(N);for(int v:comp)in[v]=1;
     for(int v=0;v<N;v++)if(!in[v]){
      if(input[v/C][v%C]!='.'||!(dom[v]&1))return false;
      if(dom[v]!=1){dom[v]=1;change=true;}
     }
    }
   }
  }
  return true;
 }
 bool dfs(vector<unsigned char> dom){
  if((++nodes%P4_CONFIG::CLOCK_CHECK_NODES==0)&&timed()){expired=true;return false;}
  if(!propagate(dom))return false;
  int chosen=-1,best=INT_MAX;
  for(int v=0;v<N;v++){
   int count=__builtin_popcount((unsigned)dom[v]);if(count<=1)continue;
   int rank=count+20;
   if(input[v/C][v%C]!='.')rank=count;
   else for(int u:adj[v])if(u>=0 && !(dom[u]&1)){rank=count+8;break;}
   if(rank<best){best=rank;chosen=v;}
  }
  if(chosen<0){answer=move(dom);return true;}
  for(int k=0;k<7;k++)if(dom[chosen]>>k&1){
   auto next=dom;next[chosen]=1<<k;
   if(dfs(move(next)))return true;
   if(expired)return false;
  }
  return false;
 }
 vector<string> geometric(){
  vector<int> pearls;int minr=R,maxr=-1,minc=C,maxc=-1;
  for(int v=0;v<N;v++)if(input[v/C][v%C]!='.'){
   pearls.push_back(v);minr=min(minr,v/C);maxr=max(maxr,v/C);minc=min(minc,v%C);maxc=max(maxc,v%C);
  }
  vector<int> rr,cc;
  for(int v:pearls)for(int delta=-2;delta<=2;delta++){
   if(v/C+delta>=0&&v/C+delta<R)rr.push_back(v/C+delta);
   if(v%C+delta>=0&&v%C+delta<C)cc.push_back(v%C+delta);
  }
  rr.push_back(0);rr.push_back(R-1);cc.push_back(0);cc.push_back(C-1);
  sort(rr.begin(),rr.end());rr.erase(unique(rr.begin(),rr.end()),rr.end());
  sort(cc.begin(),cc.end());cc.erase(unique(cc.begin(),cc.end()),cc.end());
  vector<pair<int,int>> verts;
  auto maskAt=[&](int r,int c){
   int m=0;
   for(int i=0;i<(int)verts.size();i++){
    auto [a,b]=verts[i];auto [x,y]=verts[(i+1)%verts.size()];
    if(a==x&&r==a&&c>=min(b,y)&&c<=max(b,y)){
     if(c>min(b,y))m|=8;if(c<max(b,y))m|=2;
    }
    if(b==y&&c==b&&r>=min(a,x)&&r<=max(a,x)){
     if(r>min(a,x))m|=1;if(r<max(a,x))m|=4;
    }
   }
   return m;
  };
  auto check=[&](){
   for(int v:pearls){
    int r=v/C,c=v%C,m=maskAt(r,c);char p=input[r][c];
    if(!m||(p=='W'&&!straight(m))||(p=='B'&&straight(m)))return false;
    bool turn=false;
    for(int d=0;d<4;d++)if(m>>d&1){
     bool st=straight(maskAt(r+dr[d],c+dc[d]));
     if(p=='B'&&!st)return false;
     if(!st)turn=true;
    }
    if(p=='W'&&!turn)return false;
   }
   return true;
  };
  auto draw=[&](){
   vector<string> out(R,string(C,'.'));
   for(int r=0;r<R;r++)for(int c=0;c<C;c++){
    int m=maskAt(r,c);for(int k=0;k<7;k++)if(masks[k]==m)out[r][c]=symbols[k];
   }
   return out;
  };
  vector<array<int,4>> boxes;
  for(int t:rr)if(t<=minr)for(int b:rr)if(b>=maxr&&b-t>=2)
   for(int l:cc)if(l<=minc)for(int r:cc)if(r>=maxc&&r-l>=2)boxes.push_back({t,b,l,r});
  sort(boxes.begin(),boxes.end(),[](auto a,auto b){return (a[1]-a[0]+a[3]-a[2])<(b[1]-b[0]+b[3]-b[2]);});
  for(auto box:boxes){auto[t,b,l,r]=box;verts={{t,l},{t,r},{b,r},{b,l}};if(check())return draw();}
  for(auto box:boxes){
   auto[t,b,l,r]=box;
   for(int h:rr)if(h>t&&h<b)for(int a:cc)if(a>l&&a<r)for(int z:cc)if(z>a&&z<r){
    if(chrono::duration<double>(chrono::steady_clock::now()-start).count()>P4_CONFIG::GEOMETRY_BUDGET_S)return {};
    verts={{t,l},{t,a},{h,a},{h,z},{t,z},{t,r},{b,r},{b,l}};
    if(check())return draw();
   }
  }
  return {};
 }
 vector<string> run(vector<string> grid){
  start=chrono::steady_clock::now();input=grid;R=grid.size();C=grid[0].size();N=R*C;
  auto quick=geometric();if(!quick.empty())return quick;
  adj.resize(N);vector<unsigned char> dom(N);
  for(int v=0;v<N;v++){
   char ch=grid[v/C][v%C];int bits=0;
   for(int k=0;k<7;k++){
    int m=masks[k];
    if(ch=='W'&&!straight(m))continue;
    if(ch=='B'&&(!m||straight(m)))continue;
    bool ok=true;
    for(int d=0;d<4;d++){
     int r=v/C+dr[d],c=v%C+dc[d];
     adj[v][d]=(r>=0&&r<R&&c>=0&&c<C)?r*C+c:-1;
     if(adj[v][d]<0&&(m>>d&1))ok=false;
    }
    if(ok)bits|=1<<k;
   }
   dom[v]=bits;
  }
  // Fill adjacency even for cells with no permitted nonempty shape.
  for(int v=0;v<N;v++)for(int d=0;d<4;d++){
   int r=v/C+dr[d],c=v%C+dc[d];adj[v][d]=(r>=0&&r<R&&c>=0&&c<C)?r*C+c:-1;
  }
  if(dfs(dom)){
   vector<string> out(R,string(C,'.'));
   for(int v=0;v<N;v++)out[v/C][v%C]=symbols[__builtin_ctz((unsigned)answer[v])];
   return out;
  }
  return vector<string>(R,string(C,'.'));
 }
};
}


// ---------- SA on the loop shape: loop = boundary of a hole-free, pinch-free set of 2x2 faces ----------
namespace loopsa {
int R, C, FR, FC;
vector<char> inF;                 // face (r,c) covers cells (r..r+1, c..c+1)
const vector<char> *pearlp;
inline int F(int r, int c) { return (r < 0 || c < 0 || r >= FR || c >= FC) ? 0 : inF[r * FC + c]; }
inline int mask(int r, int c) {   // N1 E2 S4 W8 at cell (r,c)
    int a = F(r - 1, c - 1), b = F(r - 1, c), c2 = F(r, c - 1), d = F(r, c);
    return (a ^ b) | ((b ^ d) << 1) | ((c2 ^ d) << 2) | ((a ^ c2) << 3);
}
inline bool pinch(int r, int c) {
    int a = F(r - 1, c - 1), b = F(r - 1, c), c2 = F(r, c - 1), d = F(r, c);
    return a == d && b == c2 && a != b;
}
inline bool straightM(int m) { return m == 5 || m == 10; }
const int DR4[4] = {-1, 0, 1, 0}, DC4[4] = {0, 1, 0, -1};
double pearlCost(int r, int c) {
    char p = (*pearlp)[r * C + c];
    int m = mask(r, c);
    if (m == 0) return 2;
    if (p == 'W') {
        if (!straightM(m)) return 1;
        int d1 = m == 5 ? 0 : 1, d2 = d1 + 2;
        int m1 = mask(r + DR4[d1], c + DC4[d1]), m2 = mask(r + DR4[d2], c + DC4[d2]);
        return (straightM(m1) && straightM(m2)) ? 1 : 0;
    }
    double v = straightM(m) ? 1 : 0;
    for (int d = 0; d < 4; d++) if (m >> d & 1) if (!straightM(mask(r + DR4[d], c + DC4[d]))) v += 0.5;
    return v;
}
// 8-ring simple-point test for face (r,c): F 4-connected, complement 8-connected
bool simple(int r, int c) {
    static const int rr[8] = {-1, -1, 0, 1, 1, 1, 0, -1}, cc[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    int v[8], par[8];
    for (int i = 0; i < 8; i++) { v[i] = F(r + rr[i], c + cc[i]); par[i] = i; }
    function<int(int)> fd = [&](int x) { return par[x] == x ? x : par[x] = fd(par[x]); };
    auto un = [&](int a, int b) { par[fd(a)] = fd(b); };
    for (int i = 0; i < 8; i++) {
        int j = (i + 1) & 7;
        if (v[i] == v[j]) un(i, j);
        if (!v[i] && (i % 2 == 0)) { int k = (i + 2) & 7; if (!v[k]) un(i, k); }  // complement: 8-adjacent edge neighbours
    }
    int t4 = 0, t8 = 0; bool seen[8] = {};
    for (int i = 0; i < 8; i++) {
        int root = fd(i);
        if (seen[root]) continue;
        if (v[i]) {
            bool adj4 = false;
            for (int j = 0; j < 8; j++) if (fd(j) == root && j % 2 == 0) adj4 = true;
            if (adj4) { seen[root] = true; t4++; }
        } else { seen[root] = true; t8++; }
    }
    return t4 == 1 && t8 == 1;
}
vector<string> run(const vector<string> &grid, double deadline, double t0s, double t1s, double lam,
                   chrono::steady_clock::time_point T0, uint64_t &rs, int &outLen) {
    auto el = [&]() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); };
    auto rnd = [&]() { rs ^= rs << 7; rs ^= rs >> 9; return rs; };
    R = grid.size(); C = grid[0].size(); FR = R - 1; FC = C - 1;
    static vector<char> pv; pv.assign(R * C, 0);
    vector<pair<int,int>> pearls;
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) if (grid[r][c] != '.') { pv[r * C + c] = grid[r][c]; pearls.push_back({r, c}); }
    pearlp = &pv;
    inF.assign(FR * FC, 1);
    double viol = 0; int L = 0;
    for (auto [r, c] : pearls) viol += pearlCost(r, c);
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) L += __builtin_popcount(mask(r, c));
    L /= 2;
    int nF = FR * FC;
    vector<string> best; int bestL = INT_MAX;
    double minViol = 1e18;
    long long it = 0; double T = t0s, start = el();
    vector<int> near;
    while (true) {
        if ((it & 1023) == 0) {
            double now = el();
            if (now > deadline) break;
            double f = (now - start) / max(1e-9, deadline - start);
            T = t0s * pow(t1s / t0s, f);
        }
        it++;
        int fr = rnd() % FR, fc = rnd() % FC;
        int cur = inF[fr * FC + fc];
        if (cur && nF == 1) continue;
        // only faces touching the boundary can be toggled simply
        if (!simple(fr, fc)) continue;
        // affected pearls: cells rows fr-1..fr+2, cols fc-1..fc+2
        double before = 0;
        near.clear();
        for (int r = max(0, fr - 1); r <= min(R - 1, fr + 2); r++)
            for (int c = max(0, fc - 1); c <= min(C - 1, fc + 2); c++) if (pv[r * C + c]) { near.push_back(r * C + c); before += pearlCost(r, c); }
        int dL = 0;
        { // edges of the face: top, bottom, left, right
            int e1 = F(fr - 1, fc) ^ cur, e2 = F(fr + 1, fc) ^ cur, e3 = F(fr, fc - 1) ^ cur, e4 = F(fr, fc + 1) ^ cur;
            dL = 4 - 2 * (e1 + e2 + e3 + e4);
        }
        inF[fr * FC + fc] = !cur;
        bool ok = !pinch(fr, fc) && !pinch(fr, fc + 1) && !pinch(fr + 1, fc) && !pinch(fr + 1, fc + 1);
        double after = 0;
        if (ok) for (int x : near) after += pearlCost(x / C, x % C);
        double delta = (after - before) + lam * dL;
        if (ok && (delta <= 0 || (rnd() >> 11) * (1.0 / 9007199254740992.0) < exp(-delta / T))) {
            viol += after - before; L += dL; nF += cur ? -1 : 1;
            if (viol < minViol) minViol = viol;
            if (viol < 1e-9 && L < bestL) {
                bestL = L;
                best.assign(R, string(C, '.'));
                static const char *sym = "..........";
                (void)sym;
                for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) {
                    int m = mask(r, c); char ch = '.';
                    if (m == 5) ch = '|'; else if (m == 10) ch = '-'; else if (m == 3) ch = 'L';
                    else if (m == 9) ch = 'J'; else if (m == 6) ch = 'F'; else if (m == 12) ch = '7';
                    best[r][c] = ch;
                }
            }
        } else inF[fr * FC + fc] = cur;
    }
    outLen = bestL;
#ifdef LOCAL
    cerr << "SA minViol " << minViol << " finalViol " << viol << " iters " << it << " L " << L << "\n";
#endif
    return best;
}
}  // namespace loopsa

namespace p4b {
const int DR[4] = {-1, 0, 1, 0};  // N E S W
const int DC[4] = {0, 1, 0, -1};

chrono::steady_clock::time_point T0;
double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }
uint64_t rs;
inline uint64_t rnd() { rs ^= rs << 7; rs ^= rs >> 9; return rs; }

int R, C, NC, NE, OFFE;          // cells, edges, sentinel off edge
vector<char> pearl;              // 0, 'W', 'B'
vector<array<int, 4>> ce;        // cell -> edge per dir (OFFE if outside)
vector<array<int, 4>> nb;        // cell -> neighbour per dir (-1 if outside)
vector<int> ea, eb;              // edge endpoints
vector<int> st, deg, unk, partner, clen;
int totalOn, pearlsTouched, numPearls;
int closed;
vector<pair<int *, int>> trail;
vector<int> q;
vector<char> inq;
long long nodes, nodeLimit, bestLen;
bool aborted;
bool randomOrder;

inline void setv(int &x, int v) { trail.push_back({&x, x}); x = v; }
inline void push(int c) { if (c >= 0 && !inq[c]) { inq[c] = 1; q.push_back(c); } }
inline void pushAround(int c) { push(c); for (int d = 0; d < 4; d++) push(nb[c][d]); }

bool setEdge(int e, int v) {
    if (st[e] == v) return true;
    if (st[e] != -1) return false;
    int a = ea[e], b = eb[e];
    setv(st[e], v);
    setv(unk[a], unk[a] - 1); setv(unk[b], unk[b] - 1);
    if (v == 1) {
        if (deg[a] >= 2 || deg[b] >= 2) return false;
        int endA = deg[a] == 0 ? a : partner[a], lenA = deg[a] == 0 ? 0 : clen[a];
        int endB = deg[b] == 0 ? b : partner[b], lenB = deg[b] == 0 ? 0 : clen[b];
        int touched = pearlsTouched + (deg[a] == 0 && pearl[a]) + (deg[b] == 0 && pearl[b]);
        if (endA == b) {  // closing a loop
            if (!(totalOn == lenA && touched == numPearls)) return false;
            setv(closed, 1);
        } else {
            setv(partner[endA], endB); setv(partner[endB], endA);
            setv(clen[endA], lenA + lenB + 1); setv(clen[endB], lenA + lenB + 1);
            push(endA); push(endB);
        }
        setv(deg[a], deg[a] + 1); setv(deg[b], deg[b] + 1);
        setv(totalOn, totalOn + 1);
        setv(pearlsTouched, touched);
        if (bestLen < INT_MAX && P4CFG::BB && totalOn >= bestLen) return false;
    }
    pushAround(a); pushAround(b);
    return true;
}

inline int E(int c, int d) { return ce[c][d]; }
inline int S(int c, int d) { return st[ce[c][d]]; }

bool checkCell(int c) {
    int dg = deg[c], uk = unk[c];
    if (dg > 2) return false;
    if (dg == 1 && uk == 0) return false;
    if (pearl[c] && dg + uk < 2) return false;
    if (dg == 2 && uk > 0) { for (int d = 0; d < 4; d++) if (S(c, d) == -1 && !setEdge(E(c, d), 0)) return false; }
    else if (dg == 1 && uk == 1) { for (int d = 0; d < 4; d++) if (S(c, d) == -1 && !setEdge(E(c, d), 1)) return false; }
    else if (dg == 0 && uk == 1 && !pearl[c]) { for (int d = 0; d < 4; d++) if (S(c, d) == -1 && !setEdge(E(c, d), 0)) return false; }
    else if (pearl[c] && dg + uk == 2 && uk > 0) { for (int d = 0; d < 4; d++) if (S(c, d) == -1 && !setEdge(E(c, d), 1)) return false; }
    if (pearl[c] == 'B') {
        for (int d = 0; d < 4; d++) {
            int o = (d + 2) & 3;
            int n1 = nb[c][d];
            int far = n1 >= 0 ? E(n1, d) : OFFE;   // edge continuing straight beyond the neighbour
            if (S(c, d) == 1) {
                if (!setEdge(E(c, o), 0)) return false;
                if (!setEdge(far, 1)) return false;
            }
            if (st[far] == 0 && !setEdge(E(c, d), 0)) return false;
            // the arm's cell must go straight: a sideways edge there kills this arm
            if (n1 >= 0 && (S(n1, (d + 1) & 3) == 1 || S(n1, (d + 3) & 3) == 1) && !setEdge(E(c, d), 0)) return false;
            if (S(c, d) == 0 && !setEdge(E(c, o), 1)) return false;
        }
    } else if (pearl[c] == 'W') {
        // straight: E==W, N==S, horizontal != vertical
        for (int d = 0; d < 4; d++) {
            int o = (d + 2) & 3, p = (d + 1) & 3, po = (d + 3) & 3;
            int s = S(c, d);
            if (s == 1) { if (!setEdge(E(c, o), 1) || !setEdge(E(c, p), 0) || !setEdge(E(c, po), 0)) return false; }
            if (s == 0) { if (!setEdge(E(c, o), 0) || !setEdge(E(c, p), 1) || !setEdge(E(c, po), 1)) return false; }
        }
        // an orientation whose two far cells both continue straight is impossible
        for (int d = 0; d < 2; d++) {
            int o = d + 2;
            int n1 = nb[c][d], n2 = nb[c][o];
            if (n1 < 0 || n2 < 0) continue;
            if (S(n1, d) == 1 && S(n2, o) == 1 && !setEdge(E(c, d), 0)) return false;
        }
        // must turn in at least one neighbour along the loop
        for (int d = 0; d < 4; d++) {
            int o = (d + 2) & 3;
            if (S(c, d) != 1) continue;
            int n1 = nb[c][d], n2 = nb[c][o];
            int far1 = E(n1, d), far2 = E(n2, o);
            if (st[far1] == 1 && !setEdge(far2, 0)) return false;
            if (st[far2] == 1 && !setEdge(far1, 0)) return false;
        }
    }
    // no premature loop: chain end next to its partner
    if (deg[c] == 1) {
        int p = partner[c];
        for (int d = 0; d < 4; d++) if (nb[c][d] == p && S(c, d) == -1) {
            bool okClose = totalOn == clen[c] && pearlsTouched == numPearls;
            if (!okClose && !setEdge(E(c, d), 0)) return false;
        }
    }
    return true;
}

bool propagate() {
    while (!q.empty()) {
        int c = q.back(); q.pop_back(); inq[c] = 0;
        if (!checkCell(c)) { for (int x : q) inq[x] = 0; q.clear(); return false; }
    }
    return true;
}

void undo(size_t sz) { while (trail.size() > sz) { *trail.back().first = trail.back().second; trail.pop_back(); } }

bool finish() {  // loop closed: everything else off
    for (int e = 0; e < NE; e++) if (st[e] == -1 && !setEdge(e, 0)) return false;
    return propagate();
}

vector<int> stampv, bfsq;
int stampId = 0;
// all pearls and loop cells must lie in one component of the not-off edge graph
bool connected() {
    if ((int)stampv.size() != NC) stampv.assign(NC, 0);
    int src = -1;
    for (int c = 0; c < NC && src < 0; c++) if (pearl[c] || deg[c] > 0) src = c;
    if (src < 0) return true;
    ++stampId;
    bfsq.clear(); bfsq.push_back(src); stampv[src] = stampId;
    for (size_t h = 0; h < bfsq.size(); h++) {
        int c = bfsq[h];
        for (int d = 0; d < 4; d++) if (S(c, d) != 0) {
            int n1 = nb[c][d];
            if (stampv[n1] != stampId) { stampv[n1] = stampId; bfsq.push_back(n1); }
        }
    }
    for (int c = 0; c < NC; c++) if ((pearl[c] || deg[c] > 0) && stampv[c] != stampId) return false;
    return true;
}

// failed-literal probing over edges in `es` (or all edges if es empty) until no change
bool probe(const vector<int> *es) {
    bool changed = true;
    int rounds = 0;
    while (changed && rounds++ < P4CFG::PROBE_ROUNDS) {
        changed = false;
        int cntE = es ? es->size() : NE;
        for (int idx = 0; idx < cntE; idx++) {
            int e = es ? (*es)[idx] : idx;
            if (st[e] != -1 || closed) continue;
            for (int v = 0; v <= 1; v++) {
                size_t sz = trail.size();
                bool ok = setEdge(e, v) && propagate() && (closed || !P4CFG::CONN || connected());
                if (ok && closed) ok = finish();
                undo(sz);
                if (!ok) {
                    if (!setEdge(e, 1 - v) || !propagate()) return false;
                    changed = true;
                    break;
                }
            }
            if ((idx & 63) == 0 && elapsed() > P4CFG::HARD_STOP_S) return true;
        }
    }
    return true;
}

int dbgMaxOn = 0; vector<string> dbgSnap;
vector<int> tgtList, endList;
// FOCUS 2: chain end whose nearest target (untouched pearl / other fragment's end) is closest
int pickNearestEnd() {
    tgtList.clear(); endList.clear();
    for (int c = 0; c < NC; c++) {
        if (pearl[c] && deg[c] == 0) tgtList.push_back(c);
        else if (deg[c] == 1) { tgtList.push_back(c); if (unk[c] > 0) endList.push_back(c); }
    }
    if (endList.empty()) return -1;
    if ((long long)endList.size() * tgtList.size() > P4CFG::PICK_WORK) {
        for (int i = (int)endList.size() - 1; i > 0; i--) swap(endList[i], endList[rnd() % (i + 1)]);
        endList.resize(max<long long>(1, P4CFG::PICK_WORK / max<size_t>(1, tgtList.size())));
    }
    int best = -1; double bestD = 1e18;
    for (int a : endList) {
        int ar = a / C, ac = a % C, own = partner[a];
        double dmin = 1e9;
        for (int t : tgtList) if (t != a && t != own) dmin = min(dmin, (double)(abs(t / C - ar) + abs(t % C - ac)));
        if (dmin >= 1e9) dmin = 1e6;  // only its own partner left
        dmin = dmin * 4 + unk[a];
        if (randomOrder) dmin += 3.0 * (rnd() >> 11) * (1.0 / 9007199254740992.0);
        if (dmin < bestD) { bestD = dmin; best = a; }
    }
    return best;
}
bool dfs(int focus = -1) {
    if (!propagate()) return false;
#ifdef LOCAL
    static const bool snapOn = getenv("SNAP") != nullptr;
    if (snapOn && totalOn > dbgMaxOn) {
        dbgMaxOn = totalOn; dbgSnap.assign(R, string(C, ' '));
        for (int c = 0; c < NC; c++) {
            bool n = S(c, 0) == 1, e = S(c, 1) == 1, s2 = S(c, 2) == 1, w = S(c, 3) == 1;
            char ch = unk[c] == 0 && deg[c] == 0 ? '.' : '?';
            if (n && s2) ch = '|'; else if (e && w) ch = '-'; else if (n && e) ch = 'L'; else if (n && w) ch = 'J';
            else if (s2 && e) ch = 'F'; else if (s2 && w) ch = '7'; else if (deg[c] == 1) ch = '*';
            if (pearl[c] && deg[c] == 0) ch = pearl[c] == 'W' ? 'o' : 'x';
            dbgSnap[c / C][c % C] = ch;
        }
    }
#endif
    if (P4CFG::CONN && !closed && !connected()) return false;
    if (closed) return finish();
    if (++nodes > nodeLimit) { aborted = true; return false; }
    if ((nodes & 63) == 0 && elapsed() > P4CFG::HARD_STOP_S) { aborted = true; return false; }
    // PEARL_FIRST: settle pearls with undecided edges first (fewest unknowns), binary on one edge
    if (P4CFG::PEARL_FIRST) {
        int pc = -1, pu = INT_MAX;
        for (int c = 0; c < NC; c++) if (pearl[c] && unk[c] > 0) {
            int u = unk[c] * 8 + (randomOrder ? (int)(rnd() & 7) : 0);
            if (u < pu) { pu = u; pc = c; }
        }
        if (pc >= 0) {
            int ds2[4], k2 = 0;
            for (int d = 0; d < 4; d++) if (S(pc, d) == -1) ds2[k2++] = d;
            int d = ds2[randomOrder ? rnd() % k2 : 0];
            int e = E(pc, d);
            int first = randomOrder ? (int)(rnd() & 1) : 1;
            for (int t = 0; t < 2; t++) {
                int v = t == 0 ? first : 1 - first;
                size_t sz = trail.size();
                if (setEdge(e, v) && dfs(-1)) return true;
                undo(sz);
                if (aborted) return false;
            }
            return false;
        }
    }
    // branch cell: chain end with fewest unknown edges; else a pearl with unknown edges
    int bc = -1;
    long long bu = LLONG_MIN;
    if (P4CFG::FOCUS == 2) {
        bc = (focus >= 0 && deg[focus] == 1 && unk[focus] > 0) ? focus : pickNearestEnd();
    } else
    for (int c = 0; c < NC; c++) if (deg[c] == 1 && unk[c] > 0) {
        // FOCUS: 1 = extend the longest chain (keeps one path heading to its target),
        //        0 = chain end with the fewest unknown edges
        long long u = P4CFG::FOCUS ? (long long)clen[c] * 16 - unk[c] * 4 : -(long long)unk[c] * 4;
        if (randomOrder) u += (long long)(rnd() & 3);
        if (u > bu || bc < 0) { bu = u; bc = c; }
    }
    if (bc < 0) for (int c = 0; c < NC; c++) if (pearl[c] && unk[c] > 0) { bc = c; break; }
    if (bc < 0) return false;
    if (P4CFG::PROBE_NODE) {
        static vector<int> es;
        es.clear();
        for (int d = 0; d < 4; d++) if (S(bc, d) == -1) es.push_back(E(bc, d));
        size_t sz0 = trail.size(); (void)sz0;
        if (!probe(&es)) return false;
        if (closed) return finish();
        if (P4CFG::CONN && !connected()) return false;
        if (deg[bc] == 2 || unk[bc] == 0) return dfs();
    }
    int ds[4], k = 0;
    for (int d = 0; d < 4; d++) if (S(bc, d) == -1) ds[k++] = d;
    // order: towards the nearest untouched pearl, or the chain's partner end once all are touched
    {
        int br = bc / C, bcc = bc % C, tr = -1, tc = -1, bestD = INT_MAX;
        // target: nearest untouched pearl or end of ANOTHER fragment; own partner only if alone
        int own = deg[bc] == 1 ? partner[bc] : -1;
        for (int c = 0; c < NC; c++) {
            bool tgt = (pearl[c] && deg[c] == 0) || (deg[c] == 1 && c != bc && c != own);
            if (!tgt) continue;
            int dd = abs(c / C - br) + abs(c % C - bcc);
            if (dd < bestD) { bestD = dd; tr = c / C; tc = c % C; }
        }
        if (tr < 0 && own >= 0) { tr = own / C; tc = own % C; }
        double key[4];
        for (int i = 0; i < k; i++) {
            int n1 = nb[bc][ds[i]];
            key[i] = tr >= 0 ? abs(n1 / C - tr) + abs(n1 % C - tc) : 0;
            // ORDER: 1 = Warnsdorff (neighbour with fewest open edges first), distance breaks ties
            if (P4CFG::ORDER == 1) key[i] = (deg[n1] == 1 ? 0 : unk[n1]) * 100.0 + key[i] * 0.01;
            if (randomOrder) key[i] += P4CFG::ORDER_NOISE * (rnd() >> 11) * (1.0 / 9007199254740992.0);
        }
        for (int i = 1; i < k; i++) for (int j = i; j > 0 && key[j] < key[j - 1]; j--) { swap(key[j], key[j - 1]); swap(ds[j], ds[j - 1]); }
    }
    // try each unknown edge ON in turn (others implied off by degree when bc is a chain end)
    for (int i = 0; i < k; i++) {
        size_t sz = trail.size();
        int n1 = nb[bc][ds[i]];
        bool joined = deg[n1] >= 1;
        int own = deg[bc] == 1 ? partner[bc] : -1;
        bool ok = setEdge(E(bc, ds[i]), 1);
        // previous options are off in this branch
        for (int j = 0; ok && j < i; j++) ok = setEdge(E(bc, ds[j]), 0);
        int nextFocus = -1;
        if (ok && !joined && own >= 0 && deg[own] == 1) nextFocus = partner[own];
        if (ok && dfs(nextFocus)) return true;
        undo(sz);
        if (aborted) return false;
    }
    if (deg[bc] == 0) {  // a pearl branch could also be... pearls need degree 2, so no "all off"
        return false;
    }
    return false;
}

vector<string> render() {
    vector<string> out(R, string(C, '.'));
    for (int c = 0; c < NC; c++) {
        if (deg[c] != 2) continue;
        bool n = S(c, 0) == 1, e = S(c, 1) == 1, s = S(c, 2) == 1, w = S(c, 3) == 1;
        char ch = '.';
        if (n && s) ch = '|'; else if (e && w) ch = '-';
        else if (n && e) ch = 'L'; else if (n && w) ch = 'J';
        else if (s && e) ch = 'F'; else if (s && w) ch = '7';
        out[c / C][c % C] = ch;
    }
    return out;
}

vector<string> run(vector<string> grid) {
    T0 = chrono::steady_clock::now();
    rs = 88172645463325252ULL;
    for (auto &row : grid) {
        string t;
        for (char ch : row) if (ch == '.' || ch == 'W' || ch == 'B') t += ch;
        row = t;
    }
    while (!grid.empty() && grid.back().empty()) grid.pop_back();
    R = grid.size(); C = 0;
    for (auto &row : grid) C = max(C, (int)row.size());
    for (auto &row : grid) row.resize(C, '.');
    NC = R * C;
    pearl.assign(NC, 0);
    numPearls = 0;
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) if (grid[r][c] != '.') { pearl[r * C + c] = grid[r][c]; numPearls++; }
    // edges
    ce.assign(NC, {}); nb.assign(NC, {});
    ea.clear(); eb.clear();
    vector<int> eid(NC * 4, -1);
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) {
        int id = r * C + c;
        for (int d = 0; d < 4; d++) {
            int rr = r + DR[d], cc = c + DC[d];
            nb[id][d] = (rr >= 0 && rr < R && cc >= 0 && cc < C) ? rr * C + cc : -1;
        }
    }
    for (int c = 0; c < NC; c++) for (int d = 1; d <= 2; d++) if (nb[c][d] >= 0) {
        int e = ea.size(); ea.push_back(c); eb.push_back(nb[c][d]);
        eid[c * 4 + d] = e; eid[nb[c][d] * 4 + ((d + 2) & 3)] = e;
    }
    NE = ea.size(); OFFE = NE;
    for (int c = 0; c < NC; c++) for (int d = 0; d < 4; d++) ce[c][d] = eid[c * 4 + d] >= 0 ? eid[c * 4 + d] : OFFE;
    // initial state
    st.assign(NE + 1, -1); st[OFFE] = 0;
    deg.assign(NC, 0); unk.assign(NC, 0);
    for (int c = 0; c < NC; c++) for (int d = 0; d < 4; d++) if (ce[c][d] != OFFE) unk[c]++;
    partner.assign(NC, 0); clen.assign(NC, 0);
    for (int c = 0; c < NC; c++) partner[c] = c;
    totalOn = 0; pearlsTouched = 0; closed = 0;
    trail.clear(); q.clear(); inq.assign(NC, 0);
    for (int c = 0; c < NC; c++) push(c);
    bestLen = INT_MAX;
    vector<string> best(R, string(C, '.'));
    bool geoFound = false;
    {
        p4builder::Solver gs;
        gs.start = T0; gs.input = grid; gs.R = R; gs.C = C; gs.N = NC;
        vector<string> g = gs.geometric();
        if (!g.empty()) {
            int len = 0;
            for (auto &row : g) for (char ch : row) len += ch != '.';
            if (len > 0) { geoFound = true; bestLen = len; best = g; }
        }
    }
#ifdef LOCAL
    if (getenv("SAONLY")) {
        int L2 = INT_MAX;
        auto g2 = loopsa::run(grid, atof(getenv("SAONLY")), P4CFG::SA_T0, P4CFG::SA_T1, P4CFG::SA_LAMBDA, T0, rs, L2);
        cerr << "SA len " << L2 << " t " << elapsed() << "\n";
        return g2.empty() ? best : g2;
    }
#endif
    bool ok0 = propagate();
    if (ok0 && P4CFG::PROBE_ROOT) ok0 = probe(nullptr) && propagate();
#ifdef LOCAL
    if (getenv("WIT")) {  // debug: force the witness loop and report the first failure
        ifstream f(getenv("WIT"));
        vector<string> w(R);
        for (auto &row : w) getline(f, row);
        cerr << "initial propagate ok=" << ok0 << "\n";
        map<char, int> mk = {{'|', 5}, {'-', 10}, {'L', 3}, {'J', 9}, {'F', 6}, {'7', 12}, {'.', 0}};
        for (int c = 0; c < NC && ok0; c++) {
            int m = mk[w[c / C][c % C]];
            for (int d = 0; d < 4; d++) {
                int e = E(c, d);
                if (e == OFFE) continue;
                if (!setEdge(e, (m >> d) & 1) || !propagate() || (!closed && !connected())) { cerr << "FAIL at cell " << c / C << "," << c % C << " dir " << d << " totalOn " << totalOn << " closed " << closed << "\n"; ok0 = false; break; }
            }
        }
        cerr << "witness ok=" << ok0 << " closed=" << closed << "\n";
        exit(0);
    }
#endif
    size_t base = trail.size();
#ifdef LOCAL
    if (getenv("DBG")) { int u = 0, on = 0; for (int e = 0; e < NE; e++) { u += st[e] == -1; on += st[e] == 1; } cerr << "root: edges " << NE << " unknown " << u << " on " << on << "\n"; }
#endif
    if (ok0 && geoFound) {
        randomOrder = true;
        while (elapsed() < P4CFG::IMPROVE_UNTIL_S) {
            nodes = 0; nodeLimit = P4CFG::RESTART_NODES; aborted = false;
            if (dfs() && totalOn < bestLen) { bestLen = totalOn; best = render(); }
            undo(base);
        }
    } else if (ok0) {
        // first loop: deterministic DFS, no node limit (up to the deadline via restarts)
        randomOrder = false; nodes = 0; nodeLimit = P4CFG::FIRST_NODES; aborted = false;
        bool found = dfs();
        long long totN = nodes; int restarts = 0;
        while (!found && elapsed() < P4CFG::FIRST_DEADLINE_S && elapsed() < P4CFG::HARD_STOP_S) {
            restarts++;
            undo(base);
            randomOrder = true; nodes = 0; aborted = false;
            nodeLimit = min<long long>((long long)(nodeLimit * P4CFG::NODE_GROWTH), 50000000);
            if (P4CFG::ALT_FOCUS) P4CFG::FOCUS ^= 1;
            found = dfs();
        }
        totN += nodes;
#ifdef LOCAL
        if (getenv("DBG")) cerr << "first phase: restarts " << restarts << " last nodes " << nodes << " found " << found << " t " << elapsed() << "\n";
#endif
        if (found) { bestLen = totalOn; best = render(); }
        undo(base);
        // improvement: randomised restarts with branch and bound on length
        randomOrder = true;
        while (found && elapsed() < P4CFG::IMPROVE_UNTIL_S) {
            nodes = 0; nodeLimit = P4CFG::RESTART_NODES; aborted = false;
            if (dfs() && totalOn < bestLen) { bestLen = totalOn; best = render(); }
            undo(base);
        }
    }
#ifdef LOCAL
    if (getenv("SNAP")) { cerr << "deepest totalOn " << dbgMaxOn << "\n"; for (auto &row : dbgSnap) cerr << row << "\n"; }
    cerr << "R " << R << " C " << C << " pearls " << numPearls << " len " << bestLen << " t " << elapsed() << "\n";
#endif
    return best;
}
}  // namespace p4b

vector<string> solve(vector<string> grid) { return p4b::run(grid); }

#ifdef LOCAL
int main() {
    if (getenv("IMP")) P4CFG::IMPROVE_UNTIL_S = atof(getenv("IMP"));
    if (getenv("ON")) P4CFG::ORDER_NOISE = atof(getenv("ON"));
    if (getenv("CN")) P4CFG::CONN = atoi(getenv("CN"));
    if (getenv("AF")) P4CFG::ALT_FOCUS = atoi(getenv("AF"));
    if (getenv("PRR")) P4CFG::PROBE_ROOT = atoi(getenv("PRR"));
    if (getenv("PRN")) P4CFG::PROBE_NODE = atoi(getenv("PRN"));
    if (getenv("NL0")) P4CFG::FIRST_NODES = atoll(getenv("NL0"));
    if (getenv("ORD")) P4CFG::ORDER = atoi(getenv("ORD"));
    if (getenv("NG")) P4CFG::NODE_GROWTH = atof(getenv("NG"));
    if (getenv("HS")) { P4CFG::HARD_STOP_S = atof(getenv("HS")); P4CFG::FIRST_DEADLINE_S = atof(getenv("HS")); }
    if (getenv("ST0")) P4CFG::SA_T0 = atof(getenv("ST0"));
    if (getenv("ST1")) P4CFG::SA_T1 = atof(getenv("ST1"));
    if (getenv("SLAM")) P4CFG::SA_LAMBDA = atof(getenv("SLAM"));
    if (getenv("PF")) P4CFG::PEARL_FIRST = atoi(getenv("PF"));
    if (getenv("FO")) P4CFG::FOCUS = atoi(getenv("FO"));
    if (getenv("RN")) P4CFG::RESTART_NODES = atoll(getenv("RN"));
    int R;
    string line;
    if (!getline(cin, line)) return 0;
    R = stoi(line);
    vector<string> g(R);
    for (auto &row : g) getline(cin, row);
    auto out = solve(g);
    for (auto &row : out) cout << row << "\n";
}
#endif
