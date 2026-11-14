// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_SCC_TEST_SOURCE
#define SIRKIT_SCC_TEST_SOURCE "../../../Graph/SCC.cpp"
#endif
#include SIRKIT_SCC_TEST_SOURCE
#include <algorithm>
#include <cstdint>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <set>
namespace {
std::uint64_t checks=0;
void check(bool ok){++checks;if(!ok)std::abort();}
void verify(int n,const std::vector<std::pair<int,int>>&edges){
 scc_graph graph(n);for(auto[u,v]:edges)graph.add_edge(u,v);graph.build();const auto ids=graph.ids();
 Graph unique(graph.component_count()),multi(graph.component_count());std::set<std::pair<int,int>> seen;
 for(auto[u,v]:edges)if(ids[u]!=ids[v]){
  multi[ids[u]].push_back(ids[v]);
  if(seen.emplace(ids[u],ids[v]).second)unique[ids[u]].push_back(ids[v]);
 }
 check(graph.condensation()==unique);check(graph.condensation(false)==multi);
 check(graph.condensation()==unique);check(graph.ids()==ids);check(graph.edge_count()==edges.size());
 auto copied=graph;graph.reset();check(copied.condensation()==unique);check(copied.condensation(false)==multi);
}
}
int main(){
 for(int n:{0,1,2,7,64})verify(n,{});
 verify(6,{{0,1},{1,0},{1,2},{1,2},{2,3},{4,5},{4,5},{2,2},{4,2}});
 std::vector<std::pair<int,int>> repeated(200000,{0,1});repeated.emplace_back(1,2);repeated.emplace_back(2,2);verify(4,repeated);
 std::mt19937_64 rng(20261103);
 for(int n=1;n<=80;++n)for(int trial=0;trial<10;++trial){
  std::vector<std::pair<int,int>>edges;
  for(int v=1;v<n;++v)if(v%3){edges.emplace_back(v-1,v);edges.emplace_back(v,v-1);}
  for(int i=0;i<8*n;++i){int u=rng()%n,v=rng()%n;if(u>v)std::swap(u,v);edges.emplace_back(u,v);if(i%3==0)edges.emplace_back(u,v);}
  verify(n,edges);std::reverse(edges.begin(),edges.end());verify(n,edges);
 }
 std::printf("SCC condensation checks=%" PRIu64 "\n",checks);
}
