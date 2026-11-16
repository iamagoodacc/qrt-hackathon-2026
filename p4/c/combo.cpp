// p4-b: Masyu by edge constraint propagation + DFS, then randomised restarts for shorter loops.
// Edge states: -1 unknown, 0 off, 1 on. Rules: cell degree 0 or 2 (pearls 2), black/white pearl
// rules, and no sub-loop: chains keep partner endpoints; closing a chain is only allowed when it
// holds every on-edge and every pearl. Branch at the chain end with the fewest unknown edges.
#include <bits/stdc++.h>
using namespace std;

namespace P4CFG {
double FIRST_DEADLINE_S = 8.0;   // keep searching for a first valid loop up to this time
double IMPROVE_UNTIL_S = 0.9;    // after a loop is found, try restarts for shorter loops until here
long long RESTART_NODES = 20000; // node limit of one randomised restart (improvement phase)
double HARD_STOP_S = 8.5;        // DFS aborts at this time whatever happens
double ORDER_NOISE = 3.0;        // random noise (cells) added to the distance ordering on restarts
int CONN = 1;                    // 1 = prune when pearls / loop cells are disconnected
int ALT_FOCUS = 1;               // 1 = alternate FOCUS between restarts while no loop is found
int FOCUS = 1;                   // branch on the longest chain's end (1) or the most constrained end (0)
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

bool dfs() {
    if (!propagate()) return false;
    if (P4CFG::CONN && !closed && !connected()) return false;
    if (closed) return finish();
    if (++nodes > nodeLimit) { aborted = true; return false; }
    if ((nodes & 1023) == 0 && elapsed() > P4CFG::HARD_STOP_S) { aborted = true; return false; }
    // branch cell: chain end with fewest unknown edges; else a pearl with unknown edges
    int bc = -1;
    long long bu = LLONG_MIN;
    for (int c = 0; c < NC; c++) if (deg[c] == 1 && unk[c] > 0) {
        // FOCUS: 1 = extend the longest chain (keeps one path heading to its target),
        //        0 = chain end with the fewest unknown edges
        long long u = P4CFG::FOCUS ? (long long)clen[c] * 16 - unk[c] * 4 : -(long long)unk[c] * 4;
        if (randomOrder) u += (long long)(rnd() & 3);
        if (u > bu || bc < 0) { bu = u; bc = c; }
    }
    if (bc < 0) for (int c = 0; c < NC; c++) if (pearl[c] && unk[c] > 0) { bc = c; break; }
    if (bc < 0) return false;
    int ds[4], k = 0;
    for (int d = 0; d < 4; d++) if (S(bc, d) == -1) ds[k++] = d;
    // order: towards the nearest untouched pearl, or the chain's partner end once all are touched
    {
        int br = bc / C, bcc = bc % C, tr = -1, tc = -1, bestD = INT_MAX;
        if (pearlsTouched < numPearls) {
            for (int c = 0; c < NC; c++) if (pearl[c] && deg[c] == 0) {
                int dd = abs(c / C - br) + abs(c % C - bcc);
                if (dd < bestD) { bestD = dd; tr = c / C; tc = c % C; }
            }
        } else if (deg[bc] == 1) { tr = partner[bc] / C; tc = partner[bc] % C; }
        double key[4];
        for (int i = 0; i < k; i++) {
            int n1 = nb[bc][ds[i]];
            key[i] = tr >= 0 ? abs(n1 / C - tr) + abs(n1 % C - tc) : 0;
            if (randomOrder) key[i] += P4CFG::ORDER_NOISE * (rnd() >> 11) * (1.0 / 9007199254740992.0);
        }
        for (int i = 1; i < k; i++) for (int j = i; j > 0 && key[j] < key[j - 1]; j--) { swap(key[j], key[j - 1]); swap(ds[j], ds[j - 1]); }
    }
    // try each unknown edge ON in turn (others implied off by degree when bc is a chain end)
    for (int i = 0; i < k; i++) {
        size_t sz = trail.size();
        bool ok = setEdge(E(bc, ds[i]), 1);
        // previous options are off in this branch
        for (int j = 0; ok && j < i; j++) ok = setEdge(E(bc, ds[j]), 0);
        if (ok && dfs()) return true;
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
    bool ok0 = propagate();
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
                if (!setEdge(e, (m >> d) & 1) || !propagate()) { cerr << "FAIL at cell " << c / C << "," << c % C << " dir " << d << " totalOn " << totalOn << " closed " << closed << "\n"; ok0 = false; break; }
            }
        }
        cerr << "witness ok=" << ok0 << " closed=" << closed << "\n";
        exit(0);
    }
#endif
    size_t base = trail.size();
    if (ok0 && geoFound) {
        randomOrder = true;
        while (elapsed() < P4CFG::IMPROVE_UNTIL_S) {
            nodes = 0; nodeLimit = P4CFG::RESTART_NODES; aborted = false;
            if (dfs() && totalOn < bestLen) { bestLen = totalOn; best = render(); }
            undo(base);
        }
    } else if (ok0) {
        // first loop: deterministic DFS, no node limit (up to the deadline via restarts)
        randomOrder = false; nodes = 0; nodeLimit = 200000; aborted = false;
        bool found = dfs();
        while (!found && elapsed() < P4CFG::FIRST_DEADLINE_S && elapsed() < P4CFG::HARD_STOP_S) {
            undo(base);
            randomOrder = true; nodes = 0; aborted = false;
            nodeLimit = min<long long>(nodeLimit * 2, 50000000);
            if (P4CFG::ALT_FOCUS) P4CFG::FOCUS ^= 1;
            found = dfs();
        }
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
    cerr << "R " << R << " C " << C << " pearls " << numPearls << " len " << bestLen << " t " << elapsed() << "\n";
#endif
    return best;
}
}  // namespace p4b


// ======================= p4-c region SA =======================
namespace CFG4 {
double TIME_BUDGET_S = 0.85;     // total SA time (first second is free)
double LAMBDA = 4.0;             // penalty per violated pearl
double T_START = 2.0;            // SA start temperature
double T_END = 0.15; 
double T_START_SEEDED = 0.8;     // start temperature when seeded from a valid CSP loop
double SA_FIRST_S = 0.4;         // phase A: SA alone from rectangles
double TOTAL_S = 0.95;           // aim to finish everything by this time when possible
double CSP_SHORT_S = 0.45;       // CSP deadline (its own clock) when SA already has a loop
double SEEDED_FRAC = 0.5;        // share of the final SA phase seeded from the CSP loop            // SA end temperature
int RESTARTS = 40;               // independent SA runs (time split evenly)
int INIT_MARGIN = 1;             // initial rectangle = pearl bounding box grown by this many cells
}

namespace msy {
int R, C, SR, SC;                  // cells R x C, squares SR x SC = (R-1) x (C-1)
vector<string> G;
vector<unsigned char> sq;          // padded (SR+2) x (SC+2), 1 = inside
inline unsigned char &S(int i, int j) { return sq[(i + 1) * (SC + 2) + (j + 1)]; }
inline int sqv(int i, int j) { return sq[(i + 1) * (SC + 2) + (j + 1)]; }
uint64_t rs = 0x2545F4914F6CDD1DULL;
inline uint64_t rnd() { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return rs; }
inline double urand() { return (rnd() >> 11) * (1.0 / 9007199254740992.0); }
chrono::steady_clock::time_point T0;
double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }

enum { DN = 1, DE = 2, DS = 4, DW = 8 };
inline int maskAt(int r, int c) {
    if (r < 0 || r >= R || c < 0 || c >= C) return 0;
    int nw = sqv(r - 1, c - 1), ne = sqv(r - 1, c), sw = sqv(r, c - 1), se = sqv(r, c);
    int m = 0;
    if (nw != ne) m |= DN;
    if (ne != se) m |= DE;
    if (sw != se) m |= DS;
    if (nw != sw) m |= DW;
    return m;
}
inline bool straight(int m) { return m == (DN | DS) || m == (DE | DW); }
inline int viol(int r, int c) {
    char p = G[r][c];
    if (p == '.') return 0;
    int m = maskAt(r, c);
    if (!m) return 1;
    // neighbours along the loop
    int a[2], b[2], k = 0;
    if (m & DN) { a[k] = r - 1; b[k++] = c; }
    if (m & DE) { a[k] = r; b[k++] = c + 1; }
    if (m & DS) { a[k] = r + 1; b[k++] = c; }
    if (m & DW) { a[k] = r; b[k++] = c - 1; }
    if (k != 2) return 1;
    bool s0 = straight(maskAt(a[0], b[0])), s1 = straight(maskAt(a[1], b[1]));
    if (p == 'W') return (!straight(m) || (s0 && s1)) ? 1 : 0;
    return (straight(m) || !s0 || !s1) ? 1 : 0;   // 'B'
}
// Yokoi 4-connectivity number of square (i,j)'s 8-ring (region excluding the square itself)
inline int yokoi(int i, int j) {
    int x[9];
    x[1] = sqv(i, j + 1); x[2] = sqv(i - 1, j + 1); x[3] = sqv(i - 1, j); x[4] = sqv(i - 1, j - 1);
    x[5] = sqv(i, j - 1); x[6] = sqv(i + 1, j - 1); x[7] = sqv(i + 1, j); x[8] = sqv(i + 1, j + 1);
    int s = 0;
    for (int k = 1; k <= 7; k += 2) { int k1 = k + 1, k2 = (k + 2 > 8) ? 1 : k + 2; s += x[k] - x[k] * x[k1] * x[k2]; }
    return s;
}
// would square (i,j), set to v, create a diagonal 2x2 pattern?
inline bool diagBad(int i, int j) {
    for (int di = -1; di <= 0; di++) for (int dj = -1; dj <= 0; dj++) {
        int a = sqv(i + di, j + dj), b = sqv(i + di, j + dj + 1), c = sqv(i + di + 1, j + dj), d = sqv(i + di + 1, j + dj + 1);
        if (a == d && b == c && a != b) return true;
    }
    return false;
}

vector<pair<int, int>> pearls;
vector<vector<int>> pearlNear;   // per square: indices of pearls whose rule may change

int loopLen() { int L = 0; for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) L += maskAt(r, c) != 0; return L; }
int totalViol() { int V = 0; for (auto &p : pearls) V += viol(p.first, p.second); return V; }

vector<string> render(const vector<unsigned char> &state) {
    auto save = sq; sq = state;
    vector<string> out(R, string(C, '.'));
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) {
        int m = maskAt(r, c); char ch = '.';
        if (m == (DN | DS)) ch = '|'; else if (m == (DE | DW)) ch = '-';
        else if (m == (DN | DE)) ch = 'L'; else if (m == (DN | DW)) ch = 'J';
        else if (m == (DS | DE)) ch = 'F'; else if (m == (DS | DW)) ch = '7';
        out[r][c] = ch;
    }
    sq = save;
    return out;
}

// ---- reusable SA pieces (p4-c combo) ----
int pr0, pr1, pc0, pc1;
vector<unsigned char> saBest; int saBestLen;
void saSetup(const vector<string> &grid) {
    G.clear();
    for (auto &row : grid) { string t; for (char ch : row) if (ch == '.' || ch == 'W' || ch == 'B') t += ch; if (!t.empty()) G.push_back(t); }
    R = G.size(); C = R ? G[0].size() : 0;
    for (auto &row : G) row.resize(C, '.');
    SR = R - 1; SC = C - 1;
    pearls.clear();
    pr0 = R; pr1 = -1; pc0 = C; pc1 = -1;
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) if (G[r][c] != '.') {
        pearls.push_back({r, c}); pr0 = min(pr0, r); pr1 = max(pr1, r); pc0 = min(pc0, c); pc1 = max(pc1, c);
    }
    pearlNear.assign(max(0, SR * SC), {});
    vector<int> pid(R * C, -1);
    for (size_t k = 0; k < pearls.size(); k++) pid[pearls[k].first * C + pearls[k].second] = k;
    for (int i = 0; i < SR; i++) for (int j = 0; j < SC; j++)
        for (int r = i - 1; r <= i + 2; r++) for (int c = j - 1; c <= j + 2; c++)
            if (r >= 0 && r < R && c >= 0 && c < C && pid[r * C + c] >= 0) pearlNear[i * SC + j].push_back(pid[r * C + c]);
    saBest.clear(); saBestLen = INT_MAX;
}
// interior squares of a valid loop (ray casting to the left over vertical loop edges)
vector<unsigned char> regionFromLoop(const vector<string> &out) {
    vector<unsigned char> reg((SR + 2) * (SC + 2), 0);
    for (int i = 0; i < SR; i++) {
        int par = 0;
        for (int j = 0; j < SC; j++) {
            char ch = out[i][j];
            if (ch == '|' || ch == 'F' || ch == '7') par ^= 1;   // has S bit: edge (i,j)-(i+1,j)
            reg[(i + 1) * (SC + 2) + (j + 1)] = par;
        }
    }
    return reg;
}
// SA runs between now and tEnd (absolute seconds since T0); init = start region or nullptr (rectangle)
void saRun(double tEndAbs, int runs, const vector<unsigned char> *init) {
    if (SR < 1 || SC < 1 || pearls.empty()) return;
    double tStart = elapsed();
    runs = max(1, runs);
    for (int run = 0; run < runs; run++) {
        double tEnd = tStart + (tEndAbs - tStart) * (run + 1) / runs;
        double tBeg = elapsed();
        if (tBeg >= tEnd) continue;
        if (init) sq = *init;
        else {
            sq.assign((SR + 2) * (SC + 2), 0);
            int a0 = max(0, pr0 - CFG4::INIT_MARGIN), a1 = min(SR - 1, max(a0, pr1 + CFG4::INIT_MARGIN - 1));
            int b0 = max(0, pc0 - CFG4::INIT_MARGIN), b1 = min(SC - 1, max(b0, pc1 + CFG4::INIT_MARGIN - 1));
            for (int i = a0; i <= a1; i++) for (int j = b0; j <= b1; j++) S(i, j) = 1;
        }
        int len = loopLen(), V = totalViol();
        if (V == 0 && len < saBestLen) { saBestLen = len; saBest = sq; }
        double Ts = init ? CFG4::T_START_SEEDED : CFG4::T_START;
        double T = Ts;
        long long it = 0;
        while (true) {
            if ((it & 1023) == 0) {
                double t = elapsed();
                if (t > tEnd) break;
                double frac = (t - tBeg) / max(1e-9, tEnd - tBeg);
                T = Ts * pow(CFG4::T_END / Ts, frac);
            }
            it++;
            int i = rnd() % SR, j = rnd() % SC;
            int v = sqv(i, j);
            if (sqv(i - 1, j) == v && sqv(i + 1, j) == v && sqv(i, j - 1) == v && sqv(i, j + 1) == v) continue;
            if (yokoi(i, j) != 1) continue;
            int dl = 0, dv = 0;
            for (int r = i; r <= i + 1; r++) for (int c = j; c <= j + 1; c++) dl -= maskAt(r, c) != 0;
            auto &pn = pearlNear[i * SC + j];
            for (int p : pn) dv -= viol(pearls[p].first, pearls[p].second);
            S(i, j) = v ^ 1;
            if (diagBad(i, j)) { S(i, j) = v; continue; }
            for (int r = i; r <= i + 1; r++) for (int c = j; c <= j + 1; c++) dl += maskAt(r, c) != 0;
            for (int p : pn) dv += viol(pearls[p].first, pearls[p].second);
            double d = dl + CFG4::LAMBDA * dv;
            if (d <= 0 || urand() < exp(-d / T)) {
                len += dl; V += dv;
                if (V == 0 && len < saBestLen) { saBestLen = len; saBest = sq; }
            } else S(i, j) = v;
        }
    }
}
}  // namespace msy



// ======================= combined entry point =======================
static int loopLenOf(const vector<string> &g) { int L = 0; for (auto &r : g) for (char ch : r) L += ch != '.'; return L; }

vector<string> solve(vector<string> grid) {
    msy::T0 = chrono::steady_clock::now();
    msy::rs = 0x2545F4914F6CDD1DULL;
    msy::saSetup(grid);
    // phase A: region SA from rectangles (finds short loops on sparse puzzles)
    msy::saRun(CFG4::SA_FIRST_S, max(1, CFG4::RESTARTS / 2), nullptr);
    bool saOk = !msy::saBest.empty();
    // phase B: edge CSP (validity on dense puzzles); short deadline if SA already has a loop
    double off = msy::elapsed();
    if (saOk) {
        P4CFG::FIRST_DEADLINE_S = CFG4::CSP_SHORT_S; P4CFG::HARD_STOP_S = CFG4::CSP_SHORT_S + 0.05;
        P4CFG::IMPROVE_UNTIL_S = CFG4::CSP_SHORT_S;
    } else {
        P4CFG::FIRST_DEADLINE_S = 7.5 - off; P4CFG::HARD_STOP_S = 8.0 - off;
        P4CFG::IMPROVE_UNTIL_S = max(0.1, CFG4::TOTAL_S - off - 0.25);
    }
    vector<string> cspOut = p4b::run(grid);
    int cspLen = loopLenOf(cspOut);
    // phase C: SA again, half seeded from the CSP loop
    double tNow = msy::elapsed();
    double tEnd = max(CFG4::TOTAL_S, tNow + 0.2);
    if (cspLen > 0) {
        auto reg = msy::regionFromLoop(cspOut);
        double tMid = tNow + (tEnd - tNow) * CFG4::SEEDED_FRAC;
        msy::saRun(tMid, 4, &reg);
    }
    if (msy::elapsed() < tEnd) msy::saRun(tEnd, max(1, CFG4::RESTARTS / 2), nullptr);
    if (!msy::saBest.empty() && (cspLen == 0 || msy::saBestLen < cspLen)) return msy::render(msy::saBest);
    if (cspLen > 0) return cspOut;
    return msy::render(msy::sq);   // nothing valid found (should not happen often)
}
