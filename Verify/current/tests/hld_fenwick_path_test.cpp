// SPDX-License-Identifier: CC0-1.0
#ifndef HLD_SOURCE
#define HLD_SOURCE "../../../Graph/Heavy_Light_Decomposition.cpp"
#endif
#ifndef FW_SOURCE
#define FW_SOURCE "../../../DataStructure/Fenwick_Tree.cpp"
#endif
#ifdef SIRKIT_HLD_FENWICK_FIRST
#include FW_SOURCE
#include HLD_SOURCE
#else
#include HLD_SOURCE
#include FW_SOURCE
#endif
#include "../vendor/atcoder/fenwicktree.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cinttypes>
#include <cstdlib>
#include <utility>
#include <cstdio>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <source_location>
#include <stdexcept>
#include <type_traits>
#include <vector>
namespace {
std::uint64_t checks=0;
void check(bool ok,const std::source_location where=std::source_location::current()){
 ++checks;if(!ok){std::fprintf(stderr,"HLD Fenwick path failed at %s:%u\n",where.file_name(),where.line());std::abort();}
}
struct Group{
 std::uint64_t x=0;
 Group()=default;explicit Group(std::uint64_t v):x(v){}
 explicit operator std::uint64_t()const{return x;}
 Group& operator+=(Group b){x+=b.x;return *this;}Group& operator-=(Group b){x-=b.x;return *this;}
 friend Group operator+(Group a,Group b){a+=b;return a;}friend Group operator-(Group a,Group b){a-=b;return a;}
 bool operator==(const Group&)const=default;
};
template<class T>struct Unsigned{using type=std::make_unsigned_t<T>;};
template<>struct Unsigned<Group>{using type=std::uint64_t;};
#if defined(__SIZEOF_INT128__)
template<>struct Unsigned<__int128>{using type=unsigned __int128;};
template<>struct Unsigned<unsigned __int128>{using type=unsigned __int128;};
#endif
#ifdef SIRKIT_HLD_FENWICK_ACCESS_DEFINED
static_assert(!reinsirk::detail::HldFenwickAccess<fenwick_tree<float>>::enabled);
static_assert(!reinsirk::detail::HldFenwickAccess<fenwick_tree<double>>::enabled);
static_assert(!reinsirk::detail::HldFenwickAccess<fenwick_tree<long double>>::enabled);
static_assert(!reinsirk::detail::HldFenwickAccess<fenwick_tree<bool>>::enabled);
static_assert(!reinsirk::detail::HldFenwickAccess<atcoder::fenwick_tree<std::int64_t>>::enabled);
#endif
Graph make_graph(int n,int shape,std::mt19937_64& rng){
 std::vector<int> labels(n);std::iota(labels.begin(),labels.end(),0);std::shuffle(labels.begin(),labels.end(),rng);Graph graph(n);
 for(int v=1;v<n;++v){if(shape==4&&v%97==0)continue;int p=shape==0?v-1:shape==1?0:shape==2?(v-1)/2:static_cast<int>(rng()%v);int a=labels[v],b=labels[p];graph[a].push_back(b);graph[b].push_back(a);}
 for(auto& adjacent:graph)std::shuffle(adjacent.begin(),adjacent.end(),rng);
 return graph;
}
struct Reference{
 std::vector<int> parent,depth,component,tin,tout,order;
 Reference(const Graph& graph,int root):parent(graph.size(),-2),depth(graph.size()),component(graph.size(),-1),tin(graph.size()),tout(graph.size()){
  int count=0;auto grow=[&](int start){std::queue<int> q;q.push(start);parent[start]=-1;
   while(!q.empty()){int v=q.front();q.pop();component[v]=count;for(int u:graph[v])if(parent[u]==-2){parent[u]=v;depth[u]=depth[v]+1;q.push(u);}}++count;};
  if(!graph.empty())grow(root);
  for(int v=0;v<static_cast<int>(graph.size());++v)if(parent[v]==-2)grow(v);
  std::vector<std::vector<int>> children(graph.size());for(int v=0;v<static_cast<int>(graph.size());++v)if(parent[v]>=0)children[parent[v]].push_back(v);
  for(int start=0;start<static_cast<int>(graph.size());++start)if(parent[start]<0){std::vector<std::pair<int,bool>> stack{{start,false}};while(!stack.empty()){auto[v,finish]=stack.back();stack.pop_back();if(finish){tout[v]=static_cast<int>(order.size());continue;}tin[v]=static_cast<int>(order.size());order.push_back(v);stack.push_back({v,true});for(int u:children[v])stack.push_back({u,false});}}
 }
 template<class U>U path(int u,int v,bool edges,const std::vector<U>& values)const{
  U result{};auto add=[&](int x){result=static_cast<U>(result+values[x]);};
  while(depth[u]>depth[v]){add(u);u=parent[u];}while(depth[v]>depth[u]){add(v);v=parent[v];}
  while(u!=v){add(u);add(v);u=parent[u];v=parent[v];}if(!edges)add(u);return result;
 }
 template<class U>U subtree(int v,bool edges,const std::vector<U>& values)const{U result{};for(int i=tin[v]+static_cast<int>(edges);i<tout[v];++i)result=static_cast<U>(result+values[order[i]]);return result;}
};
template<class T,template<class>class Range=fenwick_tree>
void scalar_case(int n,int shape,std::mt19937_64& rng){
 using U=typename Unsigned<T>::type;Graph graph=make_graph(n,shape,rng);int root=static_cast<int>(rng()%n);Reference ref(graph,root);
 auto random_value=[&](){U x=static_cast<U>(rng());if constexpr(sizeof(U)>8)x=(x<<64)|static_cast<U>(rng());return x;};
 std::vector<T> initial;std::vector<U> values;for(int v=0;v<n;++v){U x=random_value();initial.push_back(static_cast<T>(x));values.push_back(x);}
 auto tree=HeavyLightDecomposition(graph,root).template with_values<Range<T>>(initial);
 for(int step=0;step<200;++step){
  int u=static_cast<int>(rng()%n),v=static_cast<int>(rng()%n);
  if(step%7==0){U x=random_value();tree.add(u,static_cast<T>(x));values[u]=static_cast<U>(values[u]+x);check(static_cast<U>(tree.get(u))==values[u]);}
  else if(step%7==1){U x=random_value();tree.set(u,static_cast<T>(x));values[u]=x;check(static_cast<U>(tree.get(u))==x);}
  else if(step%7==2){check(static_cast<U>(tree.subtree_prod(u))==ref.subtree(u,false,values));check(static_cast<U>(tree.subtree_prod(u,true))==ref.subtree(u,true,values));}
  else if(ref.component[u]==ref.component[v]){check(static_cast<U>(tree.path_prod(u,v))==ref.path(u,v,false,values));check(static_cast<U>(tree.path_prod(u,v,true))==ref.path(u,v,true,values));}
#ifndef SKIP_RUNTIME_CHECKS
  else{bool caught=false;try{tree.path_prod(u,v);}catch(const std::invalid_argument&){caught=true;}check(caught);}
#endif
 }
 for(int v:{0,n/2,n-1}){check(static_cast<U>(tree.path_prod(v,v))==values[v]);check(static_cast<U>(tree.path_prod(v,v,true))==U{});}
 tree.set(root,static_cast<T>(~U{}));tree.add(root,static_cast<T>(U{1}));values[root]=U{};check(static_cast<U>(tree.get(root))==U{});
 auto copied=tree;copied.set(root,static_cast<T>(U{7}));check(static_cast<U>(tree.get(root))==U{});
 auto moved=std::move(copied);check(copied.empty());check(static_cast<U>(moved.get(root))==U{7});moved.build(graph,tree.values(),root);check(moved.values()==tree.values());
#ifndef SKIP_RUNTIME_CHECKS
 for(int bad:{-1,n}){bool caught=false;try{tree.path_prod(bad,root);}catch(const std::out_of_range&){caught=true;}check(caught);caught=false;try{tree.path_prod(root,bad);}catch(const std::out_of_range&){caught=true;}check(caught);}
#endif
}
template<class T>void float_fallback(std::mt19937_64& rng){
 constexpr int n=8192;auto graph=make_graph(n,3,rng);int root=712;Reference ref(graph,root);std::vector<T> initial(n);std::vector<std::uint64_t> values(n);for(int v=0;v<n;++v){values[v]=rng()%64;initial[v]=static_cast<T>(values[v]);}
 auto tree=HeavyLightDecomposition(graph,root).template with_values<fenwick_tree<T>>(initial);
 for(int step=0;step<300;++step){int u=rng()%n,v=rng()%n;check(tree.path_prod(u,v)==static_cast<T>(ref.path(u,v,false,values)));check(tree.path_prod(u,v,true)==static_cast<T>(ref.path(u,v,true,values)));}
}
}
int main(){
 std::mt19937_64 rng(202611052);int forests=0;
 for(int n:{4096,4097,8192,16384})for(int shape=0;shape<5;++shape){scalar_case<std::int64_t>(n,shape,rng);scalar_case<std::uint64_t>(n,shape,rng);scalar_case<Group>(n,shape,rng);forests+=3;}
 for(int n:{4097,8192})for(int shape:{2,3}){scalar_case<std::int8_t>(n,shape,rng);scalar_case<std::uint8_t>(n,shape,rng);forests+=2;
#if defined(__SIZEOF_INT128__)
  scalar_case<__int128>(n,shape,rng);scalar_case<unsigned __int128>(n,shape,rng);forests+=2;
#endif
 }
 scalar_case<std::int64_t,atcoder::fenwick_tree>(8192,2,rng);scalar_case<std::int64_t,atcoder::fenwick_tree>(8192,4,rng);forests+=2;
 float_fallback<float>(rng);float_fallback<double>(rng);float_fallback<long double>(rng);
 std::printf("HLD Fenwick large forests=%d checks=%" PRIu64 "\n",forests,checks);
}
