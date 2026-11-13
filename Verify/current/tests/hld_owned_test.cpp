// SPDX-License-Identifier: CC0-1.0
#ifndef HLD_SOURCE
#define HLD_SOURCE "../../../Graph/Heavy_Light_Decomposition.cpp"
#endif
#ifdef SIRKIT_HLD_OWNED_RANGES_FIRST
#include "../../../DataStructure/Fenwick_Tree.cpp"
#include "../../../DataStructure/Segment_Tree.cpp"
#include "../../../DataStructure/Lazy_Segment_Tree.cpp"
#include "../vendor/atcoder/fenwicktree.hpp"
#include "../vendor/atcoder/segtree.hpp"
#include "../vendor/atcoder/lazysegtree.hpp"
#include HLD_SOURCE
#else
#include HLD_SOURCE
#include "../../../DataStructure/Fenwick_Tree.cpp"
#include "../../../DataStructure/Segment_Tree.cpp"
#include "../../../DataStructure/Lazy_Segment_Tree.cpp"
#include "../vendor/atcoder/fenwicktree.hpp"
#include "../vendor/atcoder/segtree.hpp"
#include "../vendor/atcoder/lazysegtree.hpp"
#endif
#include HLD_SOURCE
#include <algorithm>
#include <cstdint>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <source_location>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
namespace {
using I=std::int64_t;
std::uint64_t checks=0;
void check(bool ok,const std::source_location w=std::source_location::current()){
 ++checks;if(!ok){std::fprintf(stderr,"Owned HLD failed at %s:%u\n",w.file_name(),w.line());std::abort();}
}
template<class E,class F>void expect(F&& fn){bool caught=false;try{fn();}catch(const E&){caught=true;}check(caught);}
bool fail_operation=false;
I fragile_op(I a,I b){if(fail_operation)throw std::runtime_error("deliberate operation failure");return a+b;}
I plus(I a,I b){return a+b;}I zero(){return 0;}I minop(I a,I b){return std::min(a,b);}I mine(){return I{1}<<60;}
struct Total{I sum;int count;bool operator==(const Total&)const=default;};
Total join(Total a,Total b){return {a.sum+b.sum,a.count+b.count};}Total empty_total(){return {0,0};}
struct Change{bool assign;I value;};
Total mapping(Change f,Total a){return {f.assign?f.value*a.count:a.sum+f.value*a.count,a.count};}
Change composition(Change f,Change g){return f.assign?f:Change{g.assign,g.value+f.value};}Change nochange(){return {false,0};}
using Sum=segtree<I,plus,zero>;using AclSum=atcoder::segtree<I,plus,zero>;using Min=segtree<I,minop,mine>;
using Lazy=lazy_segtree<Total,join,empty_total,Change,mapping,composition,nochange>;
using AclLazy=atcoder::lazy_segtree<Total,join,empty_total,Change,mapping,composition,nochange>;
std::string concat(std::string a,const std::string& b){return a+=b;}std::string empty_word(){return {};}
struct Letter{bool replace;char value;};
std::string letter_map(Letter f,std::string x){return f.replace?std::string(x.size(),f.value):x;}
Letter letter_compose(Letter f,Letter g){return f.replace?f:g;}Letter letter_identity(){return {false,0};}
using Words=segtree<std::string,concat,empty_word>;using AclWords=atcoder::segtree<std::string,concat,empty_word>;
using WordLazy=lazy_segtree<std::string,concat,empty_word,Letter,letter_map,letter_compose,letter_identity>;
using AclWordLazy=atcoder::lazy_segtree<std::string,concat,empty_word,Letter,letter_map,letter_compose,letter_identity>;
template<class H>concept HasAdd=requires(H& h){h.add(0,I{1});};
template<class H>concept HasApply=requires(H& h){h.apply(0,Change{});};
static_assert(HasAdd<HeavyLightValues<fenwick_tree<I>>>);
static_assert(!HasAdd<HeavyLightValues<Sum>>);
static_assert(HasApply<HeavyLightValues<Lazy>>);
static_assert(!HasApply<HeavyLightValues<Sum>>);
struct Reference{
 Graph g;std::vector<int> parent,depth,component;std::vector<int> roots;
 Reference(Graph graph,int root):g(std::move(graph)),parent(g.size(),-2),depth(g.size()),component(g.size(),-1){
  if(g.empty())return;
  auto grow=[&](int start){roots.push_back(start);parent[start]=-1;std::queue<int> q;q.push(start);
   while(!q.empty()){int v=q.front();q.pop();component[v]=static_cast<int>(roots.size())-1;
    for(int u:g[v])if(parent[u]==-2){parent[u]=v;depth[u]=depth[v]+1;q.push(u);}}};
  grow(root);for(int v=0;v<static_cast<int>(g.size());++v)if(parent[v]==-2)grow(v);
 }
 int lca(int u,int v)const{if(component[u]!=component[v])return -1;while(depth[u]>depth[v])u=parent[u];while(depth[v]>depth[u])v=parent[v];while(u!=v){u=parent[u];v=parent[v];}return u;}
 std::vector<int> path(int u,int v,bool edges=false)const{
  std::vector<int> before(g.size(),-1);std::queue<int> q;q.push(u);before[u]=u;
  while(!q.empty()&&before[v]<0){int x=q.front();q.pop();for(int y:g[x])if(before[y]<0){before[y]=x;q.push(y);}}
  if(before[v]<0)return {};
  std::vector<int> answer;for(int x=v;;x=before[x]){answer.push_back(x);if(x==u)break;}std::reverse(answer.begin(),answer.end());
  if(edges){int a=lca(u,v);answer.erase(std::find(answer.begin(),answer.end(),a));}return answer;
 }
 bool ancestor(int u,int v)const{for(int x=v;x!=-1;x=parent[x])if(x==u)return true;return false;}
 std::vector<int> subtree(int v,bool edges=false)const{std::vector<int> a;for(int u=0;u<static_cast<int>(g.size());++u)if(ancestor(v,u)&&(!edges||u!=v))a.push_back(u);return a;}
};
Graph make_graph(int n,int shape,std::mt19937_64& rng){
 Graph g(n);std::vector<int> label(n);std::iota(label.begin(),label.end(),0);std::shuffle(label.begin(),label.end(),rng);
 for(int i=1;i<n;++i){if(shape==4&&i%7==0)continue;int p=shape==0?i-1:shape==1?0:shape==2?(i-1)/2:static_cast<int>(rng()%i);int u=label[i],v=label[p];g[u].push_back(v);g[v].push_back(u);}
 for(auto& a:g)std::shuffle(a.begin(),a.end(),rng);
 return g;
}
template<class H>void topology(H& h,const Reference& r){
 check(h.size()==static_cast<int>(r.g.size()));check(h.empty()==r.g.empty());check(h.component_count()==static_cast<int>(r.roots.size()));
 for(int v=0;v<h.size();++v){check(h.parent(v)==r.parent[v]);check(h.depth(v)==r.depth[v]);check(h.component(v)==r.component[v]);check(h.root_of(v)==r.roots[r.component[v]]);check(h.vertex_at(h.index(v))==v);check(h.subtree_size(v)==static_cast<int>(r.subtree(v).size()));
  const int u=h.size()/2;const auto p=r.path(v,u);check(h.same_component(v,u)==(r.component[v]==r.component[u]));check(h.is_ancestor(v,u)==r.ancestor(v,u));check(h.lca(v,u)==r.lca(v,u));check(h.distance(v,u)==(p.empty()?-1:static_cast<int>(p.size())-1));
  check(h.kth_ancestor(v,r.depth[v])==r.roots[r.component[v]]);check(h.kth_ancestor(v,r.depth[v]+1)==-1);
  for(int k:{0,static_cast<int>(p.size()/2),static_cast<int>(p.size())})check(h.jump(v,u,k)==(k<static_cast<int>(p.size())?p[k]:-1));
  std::vector<int> path;bool connected=h.for_each_path(v,u,[&](int l,int rr,bool rev){if(rev){for(int i=rr;i-->l;)path.push_back(h.vertex_at(i));}else{for(int i=l;i<rr;++i)path.push_back(h.vertex_at(i));}});check(connected==!p.empty());check(path==p);
 }
 std::vector<int> original(h.size());std::iota(original.begin(),original.end(),0);check(h.restore(h.reorder(original))==original);check(h.order().size()==original.size());check(h.root()==(r.g.empty()?-1:r.roots[0]));
}
template<class Range>void scalar_case(const Reference& r,std::mt19937_64& rng){
 std::vector<I> value(r.g.size());for(auto& x:value)x=static_cast<I>(rng()%101)-50;
 const int root=r.roots.empty()?0:r.roots[0];auto tree=HeavyLightDecomposition(r.g,root).template with_values<Range>(value);topology(tree,r);check(tree.values()==value);
 for(int it=0;it<350&&!value.empty();++it){int u=static_cast<int>(rng()%value.size()),v=static_cast<int>(rng()%value.size());bool edge=(it&1)!=0;auto p=r.path(u,v,edge);I total=0;for(int x:p)total+=value[x];
  if(r.component[u]==r.component[v])check(tree.path_prod(u,v,edge)==total);
  auto s=r.subtree(u,edge);I subtotal=0;for(int x:s)subtotal+=value[x];check(tree.subtree_prod(u,edge)==subtotal);check(tree.get(v)==value[v]);
  if(it%3==0){I x=static_cast<I>(rng()%101)-50;tree.set(v,x);value[v]=x;}else if constexpr(requires{tree.add(v,I{});}){I x=static_cast<I>(rng()%21)-10;tree.add(v,x);value[v]+=x;}
 }
 check(tree.values()==value);auto copy=tree;if(!value.empty()){copy.set(0,999);check(tree.get(0)==value[0]);}auto moved=std::move(copy);check(copy.empty());check(copy.values().empty());topology(moved,r);
 auto other=HeavyLightDecomposition(Graph{{}}).template with_values<Range>(std::vector<I>{17});tree.swap(other);check(tree.size()==1&&tree.get(0)==17);check(other.values()==value);
 other.build(r.g,value,root);check(other.values()==value);other.clear();check(other.empty()&&other.values().empty());other.build(Graph{{}},std::vector<I>{-9});check(other.get(0)==-9);
#ifndef SKIP_RUNTIME_CHECKS
 expect<std::invalid_argument>([&]{other.build(r.g,std::vector<I>(r.g.size()+1),root);});check(other.size()==1&&other.get(0)==-9);
 expect<std::out_of_range>([&]{(void)other.get(-1);});expect<std::out_of_range>([&]{other.set(1,0);});
#endif
}
void minimum_case(const Reference& r,std::mt19937_64& rng){
 std::vector<I> a(r.g.size());for(auto& x:a)x=static_cast<I>(rng()%100)-50;auto tree=HeavyLightDecomposition(r.g,r.roots.empty()?0:r.roots[0]).with_values<Min>(a);
 for(int i=0;i<100&&!a.empty();++i){int u=static_cast<int>(rng()%a.size()),v=static_cast<int>(rng()%a.size());if(r.component[u]!=r.component[v])continue;I answer=mine();for(int w:r.path(u,v,i%2))answer=std::min(answer,a[w]);check(tree.path_prod(u,v,i%2)==answer);I sub=mine();for(int w:r.subtree(u,i%2))sub=std::min(sub,a[w]);check(tree.subtree_prod(u,i%2)==sub);tree.set(v,i);a[v]=i;}
}
template<class Range,bool Ordered>void lazy_case(const Reference& r,std::mt19937_64& rng){
 std::vector<Total> a(r.g.size());std::vector<I> value(r.g.size());for(std::size_t i=0;i<a.size();++i){value[i]=static_cast<I>(rng()%100)-50;a[i]={value[i],1};}
 auto tree=HeavyLightDecomposition(r.g,r.roots.empty()?0:r.roots[0]).template with_values<Range,Ordered>(a);
 for(int it=0;it<300&&!a.empty();++it){int u=static_cast<int>(rng()%a.size()),v=static_cast<int>(rng()%a.size());bool edge=it%2;auto p=r.path(u,v,edge),sub=r.subtree(u,edge);I sum=0,ss=0;for(int x:p)sum+=value[x];for(int x:sub)ss+=value[x];if(r.component[u]==r.component[v])check(tree.path_prod(u,v,edge)==Total{sum,static_cast<int>(p.size())});check(tree.subtree_prod(u,edge)==Total{ss,static_cast<int>(sub.size())});
  Change f{it%4==0,static_cast<I>(rng()%11)-5};auto update=[&](const std::vector<int>& indices){for(int x:indices)value[x]=f.assign?f.value:value[x]+f.value;};
  switch(it%4){case 0:if(r.component[u]==r.component[v]){tree.path_apply(u,v,f,edge);update(p);}break;case 1:tree.subtree_apply(u,f,edge);update(sub);break;case 2:tree.apply(u,f);update(std::vector<int>{u});break;default:tree.set(u,{f.value,1});value[u]=f.value;}
  check(tree.get(v)==Total{value[v],1});
 }
 auto actual=tree.values();for(std::size_t i=0;i<a.size();++i)check(actual[i]==Total{value[i],1});
}
template<class Range>void word_case(const Reference& r,std::mt19937_64& rng){
 std::vector<std::string> a(r.g.size());for(std::size_t i=0;i<a.size();++i)a[i]=std::string(1,static_cast<char>('a'+i%26));
 auto tree=HeavyLightDecomposition(r.g,r.roots.empty()?0:r.roots[0]).template with_values<Range,true>(a);
 for(int it=0;it<200&&!a.empty();++it){int u=static_cast<int>(rng()%a.size()),v=static_cast<int>(rng()%a.size());bool edges=it%2;if(r.component[u]==r.component[v]){std::string answer;for(int x:r.path(u,v,edges))answer+=a[x];check(tree.path_prod(u,v,edges)==answer);}
  auto indices=r.subtree(u,edges);std::sort(indices.begin(),indices.end(),[&](int x,int y){return tree.index(x)<tree.index(y);});std::string sub;for(int x:indices)sub+=a[x];check(tree.subtree_prod(u,edges)==sub);
  if constexpr(requires{tree.path_apply(u,v,Letter{});}){Letter f{true,static_cast<char>('A'+it%26)};if(it%3==0&&r.component[u]==r.component[v]){tree.path_apply(u,v,f,edges);for(int x:r.path(u,v,edges))a[x]=std::string(a[x].size(),f.value);}else if(it%3==1){tree.subtree_apply(u,f,edges);for(int x:indices)a[x]=std::string(a[x].size(),f.value);}else {tree.apply(u,f);a[u]=std::string(a[u].size(),f.value);}}
  else{a[v]=std::string(1,static_cast<char>('A'+it%26));tree.set(v,a[v]);}
 }
 check(tree.values()==a);auto copy=tree;auto moved=std::move(copy);check(copy.empty());check(moved.values()==a);
}
template<class T,class U,template<class>class Range>void integer_boundary(){
 const auto maximum=std::numeric_limits<T>::max(),minimum=std::numeric_limits<T>::lowest();
 auto tree=HeavyLightDecomposition(Graph{{1},{0,2},{1}}).template with_values<Range<T>>(std::vector<T>{maximum,T{1},T{0}});
 check(tree.path_prod(0,2)==static_cast<T>(static_cast<U>(maximum)+U{1}));
 tree.set(0,minimum);check(tree.get(0)==minimum);tree.set(0,maximum);check(tree.get(0)==maximum);
 tree.add(0,T{1});check(tree.get(0)==static_cast<T>(static_cast<U>(maximum)+U{1}));
}
void boundaries(){
 integer_boundary<std::int8_t,std::uint8_t,fenwick_tree>();integer_boundary<std::uint8_t,std::uint8_t,fenwick_tree>();
 integer_boundary<I,std::uint64_t,fenwick_tree>();integer_boundary<std::uint64_t,std::uint64_t,fenwick_tree>();
 integer_boundary<I,std::uint64_t,atcoder::fenwick_tree>();integer_boundary<std::uint64_t,std::uint64_t,atcoder::fenwick_tree>();
#if defined(__SIZEOF_INT128__)
 integer_boundary<__int128,unsigned __int128,fenwick_tree>();integer_boundary<unsigned __int128,unsigned __int128,fenwick_tree>();
 integer_boundary<__int128,unsigned __int128,atcoder::fenwick_tree>();integer_boundary<unsigned __int128,unsigned __int128,atcoder::fenwick_tree>();
#endif
 HeavyLightValues<fenwick_tree<I>> default_tree;check(default_tree.empty()&&default_tree.values().empty());
#ifndef SKIP_RUNTIME_CHECKS
 expect<std::out_of_range>([&]{(void)default_tree.get(0);});
 expect<std::out_of_range>([&]{default_tree.add(0,1);});
#endif
 Graph alias_graph{{1,2},{0,3},{0},{1}};
 HeavyLightDecomposition alias_layout(alias_graph,2);auto expected_order=alias_layout.order();
 auto aliased_factory=std::move(alias_layout).with_values<fenwick_tree<int>>(alias_layout.order());check(aliased_factory.values()==expected_order);check(alias_layout.empty());
 HeavyLightDecomposition alias_direct(alias_graph,1);expected_order=alias_direct.order();
 HeavyLightValues<fenwick_tree<int>> aliased_direct(std::move(alias_direct),alias_direct.order());check(aliased_direct.values()==expected_order);check(alias_direct.empty());
 expected_order=aliased_direct.order();aliased_direct.build(alias_graph,aliased_direct.order(),2);check(aliased_direct.values()==expected_order);
 HeavyLightDecomposition alias_const(alias_graph,3);expected_order=alias_const.order();
 auto aliased_copy=alias_const.with_values<fenwick_tree<int>>(alias_const.order());check(aliased_copy.values()==expected_order);check(alias_const.order()==expected_order);
 HeavyLightDecomposition layout(Graph{{1},{0}},1);const auto& const_layout=layout;
 auto first=const_layout.with_values<Sum>(std::vector<I>{1,3});check(layout.size()==2&&first.path_prod(0,1)==4);
 auto second=HeavyLightDecomposition(Graph{{}}).with_values<Sum>(std::vector<I>{9});
 second=first;second.set(0,11);check(first.get(0)==1&&second.get(0)==11);
 second=std::as_const(second);check(second.get(0)==11);auto& self=second;second=std::move(self);check(second.get(0)==11);second.swap(second);check(second.get(0)==11);auto move_source=second;second=std::move(move_source);check(move_source.empty()&&second.get(0)==11);
#ifndef SKIP_RUNTIME_CHECKS
 expect<std::out_of_range>([&]{(void)move_source.get(0);});
#endif
 second.clear();check(second.empty());
#ifndef SKIP_RUNTIME_CHECKS
 expect<std::out_of_range>([&]{second.set(0,2);});
#endif
 using Fragile=segtree<I,fragile_op,zero>;auto fragile=HeavyLightDecomposition(Graph{{}}).with_values<Fragile>(std::vector<I>{9});
 fail_operation=true;expect<std::runtime_error>([&]{fragile.build(Graph{{1},{0}},std::vector<I>{2,4});});fail_operation=false;
 check(fragile.size()==1&&fragile.get(0)==9&&fragile.path_prod(0,0)==9);
 Graph empty;auto a=HeavyLightDecomposition(empty).with_values<fenwick_tree<I>>(std::vector<I>{});check(a.empty());check(a.values().empty());a.clear();check(a.empty());
 auto h=HeavyLightDecomposition(Graph{{},{}}).with_values<fenwick_tree<I>>(std::vector<I>{5,7});check(h.path_prod(0,0)==5);check(h.path_prod(0,0,true)==0);check(h.subtree_prod(0,true)==0);
#ifndef SKIP_RUNTIME_CHECKS
 expect<std::invalid_argument>([&]{(void)h.path_prod(0,1);});expect<std::out_of_range>([&]{h.add(-1,3);});
 expect<std::invalid_argument>([]{(void)HeavyLightDecomposition(Graph{{}}).with_values<fenwick_tree<I>>(std::vector<I>{});});
 auto l=HeavyLightDecomposition(Graph{{},{}}).with_values<Lazy>(std::vector<Total>{{1,1},{2,1}});expect<std::invalid_argument>([&]{l.path_apply(0,1,Change{false,10});});check(l.values()==std::vector<Total>({{1,1},{2,1}}));
#endif
 auto wrap=HeavyLightDecomposition(Graph{{1},{0}}).with_values<fenwick_tree<I>>(std::vector<I>{std::numeric_limits<I>::max(),1});check(wrap.path_prod(0,1)==std::numeric_limits<I>::min());wrap.set(0,std::numeric_limits<I>::min());check(wrap.get(0)==std::numeric_limits<I>::min());wrap.set(0,std::numeric_limits<I>::max());check(wrap.get(0)==std::numeric_limits<I>::max());
 auto acl=HeavyLightDecomposition(Graph{{1},{0}}).with_values<atcoder::fenwick_tree<I>>(std::vector<I>{std::numeric_limits<I>::max(),1});check(acl.path_prod(0,1)==std::numeric_limits<I>::min());acl.set(0,std::numeric_limits<I>::min());check(acl.get(0)==std::numeric_limits<I>::min());acl.set(0,std::numeric_limits<I>::max());check(acl.get(0)==std::numeric_limits<I>::max());
}
}
int main(){
 std::mt19937_64 rng(0x49acd923);int cases=0;
 for(int n:{0,1,2,9,31,65})for(int shape=0;shape<5;++shape){Graph g=make_graph(n,shape,rng);Reference r(g,n?static_cast<int>(rng()%n):0);scalar_case<fenwick_tree<I>>(r,rng);scalar_case<atcoder::fenwick_tree<I>>(r,rng);scalar_case<Sum>(r,rng);scalar_case<AclSum>(r,rng);minimum_case(r,rng);lazy_case<Lazy,false>(r,rng);lazy_case<AclLazy,false>(r,rng);lazy_case<Lazy,true>(r,rng);word_case<Words>(r,rng);word_case<AclWords>(r,rng);word_case<WordLazy>(r,rng);word_case<AclWordLazy>(r,rng);++cases;}
 boundaries();std::printf("Owned HLD forests=%d checks=%" PRIu64 "\n",cases,checks);
}
