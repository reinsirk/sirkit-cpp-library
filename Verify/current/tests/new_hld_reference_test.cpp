// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef HLD_SOURCE
#define HLD_SOURCE "../../../Graph/Heavy_Light_Decomposition.cpp"
#endif
#include HLD_SOURCE
#include HLD_SOURCE
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <utility>
#include <random>
#include <vector>
int main(){
 std::mt19937_64 random(20261031);std::uint64_t checks=0;
 for(int n=0;n<=70;++n)for(int rep=0;rep<8;++rep){
  std::vector<int> labels(n);std::iota(labels.begin(),labels.end(),0);std::shuffle(labels.begin(),labels.end(),random);Graph g(n);for(int v=1;v<n;++v)if(random()%5){int p=random()%v;g[labels[v]].push_back(labels[p]);g[labels[p]].push_back(labels[v]);}for(auto&r:g)std::shuffle(r.begin(),r.end(),random);
  const int root=n?random()%n:0;HeavyLightDecomposition h(g,root);assert(h.size()==n);assert(h.empty()==(n==0));assert(h.root()==(n?root:-1));std::vector<int> count(n);
  for(int v=0;v<n;++v){assert(h.vertex_at(h.index(v))==v);++count[h.index(v)];assert(h.root_of(v)>=0);assert(h.parent(h.root_of(v))==-1);++checks;}
  for(int x:count)assert(x==1);
  for(int u=0;u<n;++u)for(int v=0;v<n;++v){
   std::vector<int> pred(n,-1);std::queue<int> q;q.push(u);pred[u]=u;while(!q.empty()){int x=q.front();q.pop();for(int y:g[x])if(pred[y]==-1){pred[y]=x;q.push(y);}}
   const bool connected=pred[v]!=-1;assert(h.same_component(u,v)==connected);std::vector<int> want;
   if(connected){for(int x=v;;x=pred[x]){want.push_back(x);if(x==u)break;}std::reverse(want.begin(),want.end());}
   int expected_lca=-1;for(int x:want)if(expected_lca==-1||h.depth(x)<h.depth(expected_lca))expected_lca=x;
   assert(h.lca(u,v)==expected_lca);assert(h.distance(u,v)==(connected?int(want.size())-1:-1));
   std::vector<int> got;const bool ok=h.for_each_path(u,v,[&](int l,int r,bool reversed){if(reversed)for(int i=r;i-->l;)got.push_back(h.vertex_at(i));else for(int i=l;i<r;++i)got.push_back(h.vertex_at(i));});assert(ok==connected);assert(got==want);
   auto parts=h.path(u,v);got.clear();for(auto[l,r,rev]:parts)if(rev)for(int i=r;i-->l;)got.push_back(h.vertex_at(i));else for(int i=l;i<r;++i)got.push_back(h.vertex_at(i));assert(got==want);
   auto edge_want=want;if(connected)edge_want.erase(std::find(edge_want.begin(),edge_want.end(),expected_lca));got.clear();h.for_each_path(u,v,[&](int l,int r,bool rev){if(rev)for(int i=r;i-->l;)got.push_back(h.vertex_at(i));else for(int i=l;i<r;++i)got.push_back(h.vertex_at(i));},true);assert(got==edge_want);
   for(int k:{-1,0,1,int(want.size()/2),int(want.size())-1,int(want.size()),1000})assert(h.jump(u,v,k)==(k>=0&&k<int(want.size())?want[k]:-1));
   assert(h.jump(u,v,std::numeric_limits<std::int64_t>::min())==-1);assert(h.jump(u,v,std::numeric_limits<std::int64_t>::max())==-1);
   bool ancestor=false;if(connected)for(int x=v;x!=-1;x=h.parent(x))if(x==u)ancestor=true;assert(h.is_ancestor(u,v)==ancestor);++checks;
  }
  for(int v=0;v<n;++v){std::vector<int> ancestors;for(int x=v;x!=-1;x=h.parent(x))ancestors.push_back(x);for(int k=-1;k<=int(ancestors.size())+1;++k)assert(h.kth_ancestor(v,k)==(k>=0&&k<int(ancestors.size())?ancestors[k]:-1));
   auto[l,r]=h.subtree_range(v);assert(r-l==h.subtree_size(v));for(int i=0;i<n;++i){bool expected=false;for(int x=i;x!=-1;x=h.parent(x))if(x==v)expected=true;assert(expected==(l<=h.index(i)&&h.index(i)<r));++checks;}auto[el,er]=h.subtree_range(v,true);assert(el==l+1&&er==r);
  }
  auto copy=h;copy.clear();assert(copy.empty());assert(h.size()==n);auto moved=std::move(h);assert(h.empty());h=moved;assert(h.order()==moved.order());h=h;
 }
#ifndef SKIP_RUNTIME_CHECKS
 {HeavyLightDecomposition h(Graph{{1},{0}});auto before=h.order();for(const Graph& g:{Graph{{0}},Graph{{1,1},{0,0}},Graph{{1,2},{0,2},{0,1}},Graph{{2},{}}}){bool caught=false;try{h.build(g);}catch(const std::exception&){caught=true;}assert(caught);assert(h.order()==before);}bool caught=false;try{h.lca(-1,0);}catch(const std::out_of_range&){caught=true;}assert(caught);
  for(auto[u,v]:{std::pair{-1,0},std::pair{0,-1},std::pair{2,0},std::pair{0,2}})
   for(std::int64_t k:{std::numeric_limits<std::int64_t>::min(),std::int64_t{0},std::numeric_limits<std::int64_t>::max()}){
    caught=false;try{(void)h.jump(u,v,k);}catch(const std::out_of_range&){caught=true;}assert(caught);assert(h.order()==before);
   }
 }
#endif
 {int n=300000;Graph g(n);for(int v=1;v<n;++v){g[v-1].push_back(v);g[v].push_back(v-1);}HeavyLightDecomposition h(g,n-1);assert(h.lca(0,n-1)==n-1);assert(h.jump(0,n-1,n-1)==n-1);assert(h.kth_ancestor(0,n-1)==n-1);assert(h.jump(n-1,0,n-1)==0);assert(h.jump(0,n-1,std::numeric_limits<std::int64_t>::max())==-1);std::size_t pieces=0;h.for_each_path(0,n-1,[&](int,int,bool){++pieces;});assert(pieces==1);++checks;}
 std::cout<<"HLD checks="<<checks<<'\n';
}
