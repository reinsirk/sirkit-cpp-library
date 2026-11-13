// SPDX-License-Identifier: CC0-1.0
// ペア点更新用。独立BFS経路と頂点配列で、非可換積・鏡像添字・所有を照合する。
#ifndef PAIR_HLD_SOURCE
#define PAIR_HLD_SOURCE "../../../Graph/Heavy_Light_Decomposition.cpp"
#endif
#ifndef PAIR_SEG_SOURCE
#define PAIR_SEG_SOURCE "../../../DataStructure/Segment_Tree.cpp"
#endif
#ifdef PAIRED_SEG_FIRST
#include PAIR_SEG_SOURCE
#include PAIR_HLD_SOURCE
#else
#include PAIR_HLD_SOURCE
#include PAIR_SEG_SOURCE
#endif
#include PAIR_HLD_SOURCE
#include PAIR_SEG_SOURCE
#include <algorithm>
#include <array>
#include <cstdint>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <queue>
#include <random>
#include <source_location>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
// private接続を持たない、外部のACL形式バックエンドを模す。
template<class S, auto op, auto e>
class TaglessSegtree {
    segtree<S, op, e> range_;
public:
    explicit TaglessSegtree(const std::vector<S>& values) : range_(values) {}
    S get(int p) const { return range_.get(p); }
    void set(int p, S value) { range_.set(p, std::move(value)); }
    S prod(int l, int r) const { return range_.prod(l, r); }
    std::vector<S> values() const { return range_.values(); }
};
namespace {
std::uint64_t checks=0,forests=0;
void check(bool ok,const std::source_location where=std::source_location::current()) {
 ++checks;if(!ok){std::fprintf(stderr,"pair reference failed %s:%u\n",where.file_name(),where.line());std::abort();}
}
constexpr std::uint64_t mod=998244353;
struct Affine {std::uint64_t a=1,b=0;bool operator==(const Affine&)const=default;};
struct Large {std::uint64_t a=1,b=0,pad=0;bool operator==(const Large&)const=default;};
struct Nontrivial {std::uint64_t a=1,b=0;~Nontrivial(){}bool operator==(const Nontrivial&)const=default;};
static_assert(sizeof(Affine)==16&&sizeof(Large)>16&&!std::is_trivially_copyable_v<Nontrivial>);
template<class T>T make(std::uint64_t a,std::uint64_t b) {T x{};x.a=a;x.b=b;return x;}
template<class T>T combine(T x,T y){return make<T>(x.a*y.a%mod,(x.b*y.a+y.b)%mod);}
template<class T>T identity(){return T{};}
Affine ref_op(const Affine& x,const Affine& y){return combine(x,y);}
int throw_after=-1;
Affine throwing_op(Affine x,Affine y){if(throw_after==0)throw std::runtime_error("planned");if(throw_after>0)--throw_after;return combine(x,y);}
using Small=segtree<Affine,combine<Affine>,identity<Affine>>;
using ConstRef=segtree<Affine,ref_op,identity<Affine>>;
using Big=segtree<Large,combine<Large>,identity<Large>>;
using Nontriv=segtree<Nontrivial,combine<Nontrivial>,identity<Nontrivial>>;
using Throw=segtree<Affine,throwing_op,identity<Affine>>;
using Tagless=TaglessSegtree<Affine,combine<Affine>,identity<Affine>>;
std::int64_t scalar_add(std::int64_t a,std::int64_t b){return a+b;}
std::int64_t scalar_zero(){return 0;}
using Scalar=segtree<std::int64_t,scalar_add,scalar_zero>;
static_assert(reinsirk::detail::HldSegmentAccess<Small>::enabled);
static_assert(reinsirk::detail::HldSegmentAccess<ConstRef>::enabled);
static_assert(!reinsirk::detail::HldSegmentAccess<Big>::enabled);
static_assert(!reinsirk::detail::HldSegmentAccess<Nontriv>::enabled);
static_assert(!reinsirk::detail::HldSegmentAccess<Tagless>::enabled);
static_assert(!reinsirk::detail::HldSegmentAccess<Scalar>::enabled);
struct Reference {
 const Graph& graph;std::vector<int> parent,depth,component,tin,tout;
 Reference(const Graph& g,int root):graph(g),parent(g.size(),-2),depth(g.size()),component(g.size(),-1),tin(g.size()),tout(g.size()) {
  int c=0;auto grow=[&](int r){std::queue<int> q;q.push(r);parent[r]=-1;component[r]=c;
   while(!q.empty()){int v=q.front();q.pop();for(int u:g[v])if(parent[u]==-2){parent[u]=v;depth[u]=depth[v]+1;component[u]=c;q.push(u);}}++c;};
  if(!g.empty())grow(root);
  for(int v=0;v<static_cast<int>(g.size());++v)if(parent[v]==-2)grow(v);
  std::vector<std::vector<int>> children(g.size());for(int v=0;v<static_cast<int>(g.size());++v)if(parent[v]>=0)children[parent[v]].push_back(v);
  int timer=0;for(int r=0;r<static_cast<int>(g.size());++r)if(parent[r]<0){std::vector<std::pair<int,bool>> stack{{r,false}};
   while(!stack.empty()){auto [v,exit]=stack.back();stack.pop_back();if(exit){tout[v]=timer;continue;}tin[v]=timer++;stack.emplace_back(v,true);for(int u:children[v])stack.emplace_back(u,false);}}
 }
 int lca(int u,int v)const{if(component[u]!=component[v])return -1;while(depth[u]>depth[v])u=parent[u];while(depth[v]>depth[u])v=parent[v];while(u!=v){u=parent[u];v=parent[v];}return u;}
 bool ancestor(int u,int v)const{return tin[u]<=tin[v]&&tin[v]<tout[u];}
 std::vector<int> path(int u,int v,bool edges)const {
  std::vector<int> before(graph.size(),-1);std::queue<int> q;q.push(u);before[u]=u;
  while(!q.empty()&&before[v]<0){int x=q.front();q.pop();for(int y:graph[x])if(before[y]<0){before[y]=x;q.push(y);}}
  if(before[v]<0)return {};
  std::vector<int> result;for(int x=v;;x=before[x]){result.push_back(x);if(x==u)break;}
  std::reverse(result.begin(),result.end());if(edges){int a=lca(u,v);result.erase(std::find(result.begin(),result.end(),a));}return result;
 }
};
Graph graph(int n,int shape,std::mt19937_64& rng){
 Graph g(n);std::vector<int> label(n);std::iota(label.begin(),label.end(),0);std::shuffle(label.begin(),label.end(),rng);
 for(int i=1;i<n;++i){if(shape==4&&i%7==0)continue;int p=shape==0?i-1:shape==1?0:shape==2?(i-1)/2:static_cast<int>(rng()%i);int u=label[i],v=label[p];g[u].push_back(v);g[v].push_back(u);}
 for(auto& adjacent:g)std::shuffle(adjacent.begin(),adjacent.end(),rng);
 return g;
}
template<class T>T path_product(const Reference& ref,const std::vector<T>& values,int u,int v,bool edges){T x{};for(int a:ref.path(u,v,edges))x=combine(x,values[a]);return x;}
template<class H,class T>void probe(H& h,const Reference& ref,const std::vector<T>& values,int u,int v,bool edges){
 if(ref.component[u]==ref.component[v]){check(h.path_prod(u,v,edges)==path_product(ref,values,u,v,edges));check(h.path_prod(v,u,edges)==path_product(ref,values,v,u,edges));}
 else {
#ifndef SKIP_RUNTIME_CHECKS
  bool threw=false;try{(void)h.path_prod(u,v,edges);}catch(const std::invalid_argument&){threw=true;}check(threw);
#else
  check(h.path_prod(u,v,edges)==T{});
#endif
 }
 check(h.get(u)==values[u]);check(h.get(v)==values[v]);check(h.path_prod(u,u,true)==T{});check(h.path_prod(u,u)==values[u]);
}
template<class Range>void exercise(const Graph& g,int root,std::mt19937_64& rng){
 using T=typename reinsirk::detail::HldOwnedRangeTraits<Range>::Value;
 ++forests;std::vector<T> values;values.reserve(g.size());for(std::size_t i=0;i<g.size();++i)values.push_back(make<T>(rng()%mod,rng()%mod));
 Reference ref(g,root);auto h=HeavyLightDecomposition(g,root).template with_values<Range,true>(values);
 check(h.values()==values);if(g.empty()){check(h.empty());auto copy=h;copy.build(g,values);check(copy.values().empty());return;}
 const int n=static_cast<int>(g.size());std::vector<int> positions{0,n-1,n/2};
 for(int p=1;p<n;p*=2){positions.push_back(p-1);positions.push_back(p);if(p+1<n)positions.push_back(p+1);if(p>n/2)break;}
 if(n<=33)for(int p=0;p<n;++p)positions.push_back(p);
 for(int k=0;k<(n<100?40:60);++k)positions.push_back(static_cast<int>(rng()%n));
 std::sort(positions.begin(),positions.end());positions.erase(std::unique(positions.begin(),positions.end()),positions.end());std::shuffle(positions.begin(),positions.end(),rng);
 int iteration=0;for(int p:positions){int u=h.vertex_at(p),v=static_cast<int>(rng()%n);T value=make<T>(rng()%mod,rng()%mod);if(iteration%5==0)value=make<T>(0,static_cast<std::uint64_t>(iteration));
  h.set(u,value);values[u]=value;probe(h,ref,values,u,v,false);probe(h,ref,values,v,u,true);
  if(iteration%11==0){T expected{};for(int x:h.order())if(ref.ancestor(u,x))expected=combine(expected,values[x]);check(h.subtree_prod(u)==expected);
   expected=T{};for(int x:h.order())if(x!=u&&ref.ancestor(u,x))expected=combine(expected,values[x]);check(h.subtree_prod(u,true)==expected);check(h.values()==values);}
  ++iteration;
 }
 for(int i=0;i<12;++i){int u=static_cast<int>(rng()%n),v=static_cast<int>(rng()%n);probe(h,ref,values,u,v,i&1);}
 auto copy=h;int x=h.vertex_at(0),y=h.vertex_at(n-1);T changed=make<T>(7,11);copy.set(x,changed);check(h.values()==values);check(copy.get(x)==changed);
 auto changed_values=values;changed_values[x]=changed;probe(copy,ref,changed_values,x,y,false);
 h.set(y,make<T>(13,17));values[y]=make<T>(13,17);check(copy.get(y)==changed_values[y]);
 auto moved=std::move(copy);check(copy.empty());check(copy.values().empty());check(moved.values()==changed_values);probe(moved,ref,changed_values,y,x,true);
 copy=std::move(moved);check(moved.empty());check(copy.values()==changed_values);
 copy.build(g,values,root);check(copy.values()==values);probe(copy,ref,values,x,y,false);
 auto rebuilt_graph=Graph{{1},{0}};std::vector<T> rebuilt_values{make<T>(3,5),make<T>(7,9)};copy.build(rebuilt_graph,rebuilt_values,1);copy.set(0,make<T>(2,4));rebuilt_values[0]=make<T>(2,4);check(copy.path_prod(1,0)==combine(rebuilt_values[1],rebuilt_values[0]));check(h.values()==values);
}
void exceptions(std::mt19937_64& rng){
 Graph g=graph(33,3,rng);std::vector<Affine> values(33);for(int i=0;i<33;++i)values[i]=make<Affine>(i+2,i+3);
 for(int budget=0;budget<=13;++budget){throw_after=-1;HeavyLightValues<Throw,true> h(g,values,17);throw_after=budget;bool threw=false;
  try{h.set(11,make<Affine>(37,41));}catch(const std::runtime_error&){threw=true;}throw_after=-1;check(threw==(budget<12));
  h.clear();h.build(g,values,17);check(h.values()==values);Reference ref(g,17);check(h.path_prod(3,31)==path_product(ref,values,3,31,false));}
#ifndef SKIP_RUNTIME_CHECKS
 HeavyLightValues<Small,true> h(g,values,17);for(int v:{-1,33}){bool threw=false;try{h.set(v,{});}catch(const std::out_of_range&){threw=true;}check(threw);check(h.values()==values);}
#endif
}
}
int main(){
 std::mt19937_64 rng(0x331ecf090b87ULL);
 for(int n:{0,1,2,3,31,32,33,4095,4096,4097,16385})for(int shape=0;shape<5;++shape){auto g=graph(n,shape,rng);int root=n?static_cast<int>(rng()%n):0;exercise<Small>(g,root,rng);if(n<=33||shape==2)exercise<ConstRef>(g,root,rng);}
 for(int n:{1,3,33,4097})for(int shape:{0,2,4}){auto g=graph(n,shape,rng);int root=static_cast<int>(rng()%n);exercise<Big>(g,root,rng);exercise<Nontriv>(g,root,rng);exercise<Tagless>(g,root,rng);}
 exceptions(rng);std::printf("paired reference checks=%" PRIu64 " forests=%" PRIu64 "\n",checks,forests);
}
