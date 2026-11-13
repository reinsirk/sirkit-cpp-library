// SPDX-License-Identifier: CC0-1.0
#ifndef MIN_COST_FLOW_SOURCE
#define MIN_COST_FLOW_SOURCE "../../../Graph/Min_Cost_Flow.cpp"
#endif
#include MIN_COST_FLOW_SOURCE
#include MIN_COST_FLOW_SOURCE
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <tuple>
#include <vector>
struct InputEdge{int u,v,capacity,cost;};
std::vector<std::int64_t> enumerate(int n,const std::vector<InputEdge>&edges,int source,int sink){
 int bound=0;for(auto e:edges)bound+=e.capacity;std::vector<std::int64_t>best(bound+1,std::numeric_limits<std::int64_t>::max()),balance(n);
 auto dfs=[&](auto&&self,std::size_t i,std::int64_t cost)->void{if(i==edges.size()){for(int v=0;v<n;++v)if(v!=source&&v!=sink&&balance[v])return;if(balance[source]>0||balance[sink]!=-balance[source])return;int value=int(balance[sink]);best[value]=std::min(best[value],cost);return;}auto e=edges[i];for(int sent=0;sent<=e.capacity;++sent){balance[e.u]-=sent;balance[e.v]+=sent;self(self,i+1,cost+sent*e.cost);balance[e.u]+=sent;balance[e.v]-=sent;}};dfs(dfs,0,0);while(best.size()>1&&best.back()==std::numeric_limits<std::int64_t>::max())best.pop_back();return best;
}
template<class Cap,class Cost>void extremes(){
 const Cost maximum=std::numeric_limits<Cost>::max(),minimum=std::numeric_limits<Cost>::min();
 {MinCostFlow<Cap,Cost> graph(2);graph.add_edge(0,1,1,maximum);auto[flow,cost]=graph.flow(0,1);assert(flow==1&&cost==maximum);}
 {MinCostFlow<Cap,Cost> graph(2);graph.add_edge(0,1,1,static_cast<Cost>(-maximum));auto[flow,cost]=graph.flow(0,1);assert(flow==1&&cost==-maximum);}
 {MinCostFlow<Cap,Cost> graph(3);graph.add_edge(0,1,1,static_cast<Cost>(-maximum));graph.add_edge(1,2,1,Cost{-1});auto[flow,cost]=graph.flow(0,2);assert(flow==1&&cost==minimum);}
 {MinCostFlow<Cap,Cost> graph(2);const Cost a=maximum/2;graph.add_edge(0,1,1,static_cast<Cost>(-a));graph.add_edge(0,1,3,a);auto[flow,cost]=graph.flow(0,1);assert(flow==4&&cost==2*a);}
 {MinCostFlow<Cap,Cost> graph(2);bool caught=false;try{graph.add_edge(0,1,1,minimum);}catch(const std::overflow_error&){caught=true;}assert(caught&&graph.edge_count()==0);}
}
int main(){
 std::mt19937_64 random(20261031);std::uint64_t checks=0;
 for(int n=2;n<=6;++n)for(int rep=0;rep<700;++rep){
  std::vector<InputEdge>edges;MinCostFlow<> graph(n);std::vector<int>potential(n);for(auto&v:potential)v=int(random()%13)-6;int m=random()%9;
  for(int e=0;e<m;++e){int u=random()%n,v=random()%n,c=int(random()%3),cost;if(rep&1){if(u==v)continue;if(u>v)std::swap(u,v);cost=int(random()%13)-6;}else cost=int(random()%5)+potential[v]-potential[u];assert(graph.add_edge(u,v,c,cost)==edges.size());edges.push_back({u,v,c,cost});}
  const int source=0,sink=n-1;auto expected=enumerate(n,edges,source,sink);const std::int64_t maximum=std::int64_t(expected.size())-1;
  for(std::int64_t limit:{0,1,2,5,100}){auto copy=graph;auto[flow,cost]=copy.flow(source,sink,limit);assert(flow==std::min(maximum,limit));assert(cost==expected[flow]);std::vector<std::int64_t>balance(n);std::int64_t actual_cost=0;auto result=copy.edges();assert(result.size()==edges.size());for(std::size_t i=0;i<edges.size();++i){auto e=result[i];assert(e.from==edges[i].u&&e.to==edges[i].v&&e.capacity==edges[i].capacity&&e.cost==edges[i].cost&&e.flow>=0&&e.flow<=e.capacity);balance[e.from]-=e.flow;balance[e.to]+=e.flow;actual_cost+=e.flow*e.cost;}assert(actual_cost==cost);for(int v=0;v<n;++v)assert(balance[v]==(v==source?-flow:v==sink?flow:0));++checks;}
  auto incremental=graph;std::int64_t cumulative=0,cost_sum=0;for(int step=0;step<maximum+2;++step){auto[f,c]=incremental.flow(source,sink,1);cumulative+=f;cost_sum+=c;assert(cumulative==std::min<std::int64_t>(step+1,maximum));assert(cost_sum==expected[cumulative]);++checks;}
  auto sloped=graph;auto points=sloped.slope(source,sink);assert(points.front()==std::make_pair(std::int64_t{0},std::int64_t{0}));assert(points.back()==std::make_pair(maximum,expected.back()));std::int64_t old_unit=std::numeric_limits<std::int64_t>::min();for(std::size_t i=1;i<points.size();++i){auto[f0,c0]=points[i-1];auto[f1,c1]=points[i];assert(f1>f0);assert((c1-c0)%(f1-f0)==0);auto unit=(c1-c0)/(f1-f0);assert(unit>old_unit);old_unit=unit;for(auto f=f0;f<=f1;++f)assert(expected[f]==c0+(f-f0)*unit);++checks;}
  auto partial=graph;auto[f0,c0]=partial.flow(source,sink,std::int64_t{2});auto rest=partial.slope(source,sink);assert(rest.back()==std::make_pair(maximum-f0,expected.back()-c0));
  auto cut=sloped.min_cut(source);assert(cut[source]&&!cut[sink]);sloped.reset_flow();assert(sloped.flow(source,sink)==std::make_pair(maximum,expected.back()));auto moved=std::move(sloped);assert(sloped.size()==0);sloped=moved;assert(sloped.flow(source,sink)==std::make_pair(std::int64_t{0},std::int64_t{0}));sloped=sloped;
 }
 for(bool disconnected:{false,true}){MinCostFlow<> graph(4);graph.add_edge(0,1,1,2);if(disconnected)graph.add_edge(2,2,1,-1);else{graph.add_edge(0,2,1,-1);graph.add_edge(2,0,1,0);}bool caught=false;try{graph.flow(0,1);}catch(const std::domain_error&){caught=true;}assert(caught);for(auto e:graph.edges())assert(e.flow==0);++checks;}
 {MinCostFlow<> graph(4);graph.add_edge(0,1,1,10);graph.add_edge(2,1,1,0);graph.add_edge(2,3,1,1);assert(graph.flow(0,1,1)==std::make_pair(std::int64_t{1},std::int64_t{10}));assert(graph.flow(1,0,1)==std::make_pair(std::int64_t{1},std::int64_t{-10}));assert(graph.get_edge(0).flow==0);assert(graph.flow(2,3,1)==std::make_pair(std::int64_t{1},std::int64_t{1}));}
 {MinCostFlow<> graph(2);auto a=graph.add_edge(0,1,1,3);assert(graph.flow(0,1)==std::make_pair(std::int64_t{1},std::int64_t{3}));graph.add_edge(0,1,2,5);assert(graph.flow(0,1)==std::make_pair(std::int64_t{2},std::int64_t{10}));graph.change_cost(a,-2);graph.reset_flow();auto points=graph.slope(0,1);assert(points.back()==std::make_pair(std::int64_t{3},std::int64_t{8}));for(std::size_t i=0;i<graph.edge_count();++i){auto e=graph.get_edge(i);graph.change_edge(i,e.capacity,0);}assert(graph.flow(0,1)==std::make_pair(std::int64_t{3},std::int64_t{8}));}
 {MinCostFlow<std::uint64_t> graph(2);auto maximum=std::numeric_limits<std::uint64_t>::max();graph.add_edge(0,1,maximum,0);assert(graph.flow(0,1)==std::make_pair(maximum,std::int64_t{0}));}
 {MinCostFlow<> graph(2);graph.add_edge(0,1,1,0);auto id=graph.add_edge(0,1,2,std::numeric_limits<std::int64_t>::max());bool caught=false;try{graph.flow(0,1);}catch(const std::overflow_error&){caught=true;}assert(caught&&graph.get_edge(0).flow==1&&graph.get_edge(id).flow==0);assert(graph.flow(0,1,1)==std::make_pair(std::int64_t{1},std::numeric_limits<std::int64_t>::max()));}
 extremes<std::int64_t,std::int64_t>();extremes<std::uint64_t,std::int64_t>();extremes<std::uint8_t,std::int8_t>();extremes<std::uint32_t,std::int32_t>();
#ifndef SKIP_RUNTIME_CHECKS
 {MinCostFlow<> graph(2);bool caught=false;try{graph.add_edge(0,1,-1,0);}catch(const std::invalid_argument&){caught=true;}assert(caught);caught=false;try{graph.flow(0,2);}catch(const std::out_of_range&){caught=true;}assert(caught);}
#endif
 {int n=200000;MinCostFlow<> graph(n);graph.reserve_edges(n);for(int i=1;i<n;++i)graph.add_edge(i-1,i,1,1);assert(graph.flow(0,n-1)==std::make_pair(std::int64_t{1},std::int64_t{n-1}));++checks;}
 std::cout<<"MinCostFlow checks="<<checks<<'\n';
}
