// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef LOW_LINK_SOURCE
#define LOW_LINK_SOURCE "../../../Graph/Low_Link.cpp"
#endif
#include LOW_LINK_SOURCE
#include LOW_LINK_SOURCE
#include <cassert>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
using Edges=std::vector<std::pair<int,int>>;
int components(int n,const Edges&edges,int removed_vertex=-1,int removed_edge=-1,const std::vector<unsigned char>*bridges=nullptr){
 std::vector<std::vector<int>>g(n);for(std::size_t e=0;e<edges.size();++e){auto[u,v]=edges[e];if(int(e)==removed_edge||u==removed_vertex||v==removed_vertex||(bridges&&(*bridges)[e]))continue;g[u].push_back(v);g[v].push_back(u);}int count=0;std::vector<unsigned char>seen(n);for(int root=0;root<n;++root)if(root!=removed_vertex&&!seen[root]){++count;std::vector<int>stack{root};seen[root]=1;while(!stack.empty()){int v=stack.back();stack.pop_back();for(int u:g[v])if(!seen[u]){seen[u]=1;stack.push_back(u);}}}return count;
}
int main(){
 std::mt19937_64 random(20261031);std::uint64_t checks=0;
 for(int n=0;n<=18;++n)for(int rep=0;rep<140;++rep){
  Edges edges;UndirectedLowLink graph(n);const int m=n?int(random()%40):0;for(int i=0;i<m;++i){int u=random()%n,v=random()%n;assert(graph.add_edge(u,v)==edges.size());edges.emplace_back(u,v);}const int expected=components(n,edges);assert(graph.build()==expected);std::vector<unsigned char>bridge(m);
  for(int e=0;e<m;++e){bridge[e]=components(n,edges,-1,e)>expected;assert(graph.is_bridge(e)==bool(bridge[e]));assert(graph.edge(e)==edges[e]);++checks;}
  for(int v=0;v<n;++v){assert(graph.is_articulation(v)==(components(n,edges,v)>expected));++checks;}
  auto groups=graph.two_edge_components();assert(int(groups.size())==components(n,edges,-1,-1,&bridge));assert(graph.two_edge_component_count()==components(n,edges,-1,-1,&bridge));std::vector<int> seen(n),id(n,-1);for(std::size_t c=0;c<groups.size();++c)for(int v:groups[c]){++seen[v];id[v]=int(c);assert(graph.two_edge_component(v)==int(c));}
  for(int v=0;v<n;++v)assert(seen[v]==1);for(int e=0;e<m;++e){auto[u,v]=edges[e];assert((id[u]!=id[v])==bool(bridge[e]));++checks;}
  auto forest=graph.bridge_forest();std::size_t half=0;for(const auto&row:forest)half+=row.size();assert(half==2*graph.bridges().size());assert(graph.component_count()==expected);
  graph.build(true);std::vector<int> edge_seen(m);for(const auto&block:graph.biconnected_edge_components())for(auto e:block){assert(e<std::size_t(m));++edge_seen[e];}for(int c:edge_seen)assert(c==1);
  auto vertices=graph.biconnected_vertex_components(false);assert(vertices.size()==graph.biconnected_edge_components().size());for(std::size_t b=0;b<vertices.size();++b){std::vector<int>want;for(auto e:graph.biconnected_edge_components()[b]){want.push_back(edges[e].first);want.push_back(edges[e].second);}std::sort(want.begin(),want.end());want.erase(std::unique(want.begin(),want.end()),want.end());auto got=vertices[b];std::sort(got.begin(),got.end());assert(got==want);++checks;}
  auto all_vertices=graph.biconnected_vertex_components();std::vector<unsigned char>incident(n);for(auto[u,v]:edges)incident[u]=incident[v]=1;assert(all_vertices.size()==vertices.size()+std::count(incident.begin(),incident.end(),0));
  auto copy=graph;copy.reset();assert(copy.size()==0);assert(graph.size()==n);auto moved=std::move(graph);assert(graph.size()==0);graph=moved;assert(graph.bridges()==moved.bridges());assert(graph.two_edge_component_count()==int(groups.size()));graph=graph;assert(graph.two_edge_component_count()==int(groups.size()));
 }
 // 単純閉路に同時に現れる辺を併合する独立なブロック参照。
 for(int n=1;n<=7;++n)for(int rep=0;rep<200;++rep){int m=random()%10;Edges edges;UndirectedLowLink graph(n);for(int e=0;e<m;++e){int u=random()%n,v=random()%n;edges.emplace_back(u,v);graph.add_edge(u,v);}std::vector<int>parent(m);std::iota(parent.begin(),parent.end(),0);auto leader=[&](int x){while(parent[x]!=x)x=parent[x];return x;};
  for(unsigned mask=1;mask<(1u<<m);++mask){std::vector<int>degree(n),vertices;for(int e=0;e<m;++e)if(mask>>e&1){auto[u,v]=edges[e];++degree[u];++degree[v];}bool cycle=true;for(int v=0;v<n;++v){if(degree[v]!=0&&degree[v]!=2)cycle=false;if(degree[v])vertices.push_back(v);}if(!cycle)continue;std::vector<unsigned char>seen(n);std::vector<int>stack{vertices[0]};seen[vertices[0]]=1;while(!stack.empty()){int v=stack.back();stack.pop_back();for(int e=0;e<m;++e)if(mask>>e&1){auto[a,b]=edges[e];int u=a==v?b:b==v?a:-1;if(u>=0&&!seen[u]){seen[u]=1;stack.push_back(u);}}}if(std::any_of(vertices.begin(),vertices.end(),[&](int v){return !seen[v];}))continue;int first=-1;for(int e=0;e<m;++e)if(mask>>e&1){if(first==-1)first=e;else parent[leader(e)]=leader(first);}}
  graph.build(true);std::vector<int>block(m,-1);int number=0;for(const auto&part:graph.biconnected_edge_components()){for(auto e:part)block[e]=number;++number;}
  for(int a=0;a<m;++a)for(int b=0;b<m;++b){assert((block[a]==block[b])==(leader(a)==leader(b)));++checks;}
 }
#ifndef SKIP_RUNTIME_CHECKS
 {UndirectedLowLink g(2);bool caught=false;try{g.is_bridge(0);}catch(const std::logic_error&){caught=true;}assert(caught);g.add_edge(0,1);g.build();caught=false;try{(void)g.biconnected_edge_components();}catch(const std::logic_error&){caught=true;}assert(caught);g.build(true);caught=false;try{g.add_edge(2,0);}catch(const std::out_of_range&){caught=true;}assert(caught);assert(g.is_bridge(0));}
#endif
 {int n=300000;UndirectedLowLink g(n);g.reserve_edges(n-1);for(int v=1;v<n;++v)g.add_edge(v-1,v);assert(g.build()==1);assert(g.bridges().size()==std::size_t(n-1));assert(g.articulation_points().size()==std::size_t(n-2));assert(g.two_edge_component(0)!=g.two_edge_component(n-1));assert(g.two_edge_component_count()==n);++checks;}
 std::cout<<"LowLink checks="<<checks<<'\n';
}
