// SPDX-License-Identifier: CC0-1.0
#ifndef MAX_FLOW_SOURCE
#define MAX_FLOW_SOURCE "../../../Graph/Max_Flow.cpp"
#endif
#include MAX_FLOW_SOURCE
#include MAX_FLOW_SOURCE
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <tuple>
#include <vector>
std::int64_t reference(int n,const std::vector<std::tuple<int,int,std::int64_t>>& edges,int s,int t){
 std::vector<std::vector<std::int64_t>> c(n,std::vector<std::int64_t>(n));for(auto[u,v,w]:edges)c[u][v]+=w;std::int64_t answer=0;
 for(;;){std::vector<int> prev(n,-1);std::queue<int> q;prev[s]=s;q.push(s);while(!q.empty()&&prev[t]<0){int u=q.front();q.pop();for(int v=0;v<n;++v)if(c[u][v]&&prev[v]<0){prev[v]=u;q.push(v);}}if(prev[t]<0)return answer;std::int64_t add=std::numeric_limits<std::int64_t>::max();for(int v=t;v!=s;v=prev[v])add=std::min(add,c[prev[v]][v]);for(int v=t;v!=s;v=prev[v]){c[prev[v]][v]-=add;c[v][prev[v]]+=add;}answer+=add;}
}
int main(){
 std::mt19937_64 random(20261031);std::uint64_t checks=0;
 for(int n=2;n<=18;++n)for(int rep=0;rep<500;++rep){
  mf_graph<> graph(n);std::vector<std::tuple<int,int,std::int64_t>> edges;const int m=random()%(n*n+1);for(int i=0;i<m;++i){int u=random()%n,v=random()%n;std::int64_t c=random()%100;assert(graph.add_edge(u,v,c)==static_cast<int>(edges.size()));edges.emplace_back(u,v,c);}
  int s=random()%n,t=random()%n;while(t==s)t=random()%n;const auto expected=reference(n,edges,s,t);auto clone=graph;assert(graph.flow(s,t)==expected);assert(graph.flow(s,t)==0);std::int64_t total=0;
  for(std::int64_t limit:{0,1,7,50,1000}){auto x=clone.flow(s,t,limit);assert(x>=0&&x<=limit);total+=x;}total+=clone.flow(s,t);assert(total==expected);
  auto cut=graph.min_cut(s);assert(cut[s]&&!cut[t]);std::int64_t cut_capacity=0;std::vector<std::int64_t> balance(n);
  const auto all=graph.edges();assert(all.size()==edges.size());for(std::size_t i=0;i<edges.size();++i){auto e=graph.get_edge(static_cast<int>(i));auto[u,v,c]=edges[i];assert(e.from==u&&e.to==v&&e.cap==c&&e.flow>=0&&e.flow<=c);assert(all[i].flow==e.flow);balance[u]-=e.flow;balance[v]+=e.flow;if(cut[u]&&!cut[v])cut_capacity+=c;++checks;}
  assert(cut_capacity==expected);for(int v=0;v<n;++v)assert(balance[v]==(v==s?-expected:v==t?expected:0));
  for(std::size_t i=0;i<edges.size();++i){auto[u,v,c]=edges[i];graph.change_edge(static_cast<int>(i),c,0);}assert(graph.flow(s,t)==expected);
  auto copy=graph;graph.reset();assert(graph.size()==0);assert(copy.size()==n);auto moved=std::move(copy);assert(copy.size()==0);copy=moved;const auto& alias=copy;copy=alias;assert(copy.flow(s,t)==0);
 }
 for(int n=2;n<=8;++n)for(int rep=0;rep<200;++rep){mf_graph<> g(n);std::vector<std::tuple<int,int,std::int64_t>> es;for(int u=0;u<n;++u)for(int v=0;v<n;++v)if(random()%3==0){auto c=std::int64_t(random()%10);g.add_edge(u,v,c);es.emplace_back(u,v,c);}std::int64_t best=std::numeric_limits<std::int64_t>::max();for(unsigned mask=1;mask<(1u<<n);mask+=2)if(!(mask>>(n-1)&1)){std::int64_t sum=0;for(auto[u,v,c]:es)if((mask>>u&1)&&!(mask>>v&1))sum+=c;best=std::min(best,sum);}assert(g.flow(0,n-1)==best);++checks;}
 {mf_graph<std::uint64_t> g(2);auto maximum=std::numeric_limits<std::uint64_t>::max();g.add_edge(0,1,maximum);g.add_edge(0,1,maximum);assert(g.flow(0,1)==maximum);assert(g.flow(0,1)==maximum);assert(g.flow(0,1)==0);assert(g.get_edge(0).cap==maximum&&g.get_edge(1).cap==maximum);}
 {mf_graph<std::uint8_t> g(2);g.add_edge(0,1,255);g.add_edge(0,1,255);assert(g.flow(0,1)==255);assert(g.flow(0,1)==255);assert(g.flow(0,1)==0);}
 {mf_graph<> g(3);auto id=g.add_edge(0,1,2);g.add_edge(1,2,5);assert(g.flow(0,2)==2);g.change_edge(id,7,2);assert(g.flow(0,2)==3);assert(g.flow(0,0)==0);assert(g.flow(0,2,0)==0);}
#ifndef SKIP_RUNTIME_CHECKS
 {mf_graph<> g(2);bool caught=false;try{g.add_edge(-1,0,1);}catch(const std::out_of_range&){caught=true;}assert(caught);caught=false;try{g.add_edge(0,1,-1);}catch(const std::invalid_argument&){caught=true;}assert(caught);auto e=g.add_edge(0,1,2);caught=false;try{g.change_edge(e,1,2);}catch(const std::invalid_argument&){caught=true;}assert(caught);assert(g.get_edge(e).cap==2);caught=false;try{g.flow(0,1,-1);}catch(const std::invalid_argument&){caught=true;}assert(caught);}
#endif
 {int n=350000;mf_graph<> g(n);for(int i=1;i<n;++i)g.add_edge(i-1,i,3);assert(g.flow(0,n-1)==3);assert(g.get_edge(n/2).flow==3);++checks;}
 std::cout<<"mf_graph checks="<<checks<<'\n';
}
