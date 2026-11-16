#include <bits/stdc++.h>
using namespace std;
namespace P4_CONFIG {
constexpr double TIME_BUDGET_S=8.0;
constexpr int CLOCK_CHECK_NODES=64;
constexpr double GEOMETRY_BUDGET_S=2.0;
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
vector<string> solve(vector<string> grid){
 for(auto &row:grid)while(!row.empty()&&(row.back()=='\r'||row.back()==' '))row.pop_back();
 return p4builder::Solver().run(move(grid));
}
#ifdef LOCAL
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);int r;if(!(cin>>r))return 0;vector<string>g(r);for(auto &s:g)cin>>s;for(auto&s:solve(g))cout<<s<<'\n';}
#endif
