// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef EULERIAN_SOURCE
#define EULERIAN_SOURCE "../../../Graph/Eulerian_Trail.cpp"
#endif
#include EULERIAN_SOURCE
#include EULERIAN_SOURCE
#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
using Edges=std::vector<std::pair<int,int>>;
bool enumerate(bool directed,int n,const Edges&edges,int start,bool circuit){
 if(edges.empty())return true;auto search=[&](auto&&self,int v,unsigned used,int root)->bool{if(used==(1u<<edges.size())-1)return !circuit||v==root;for(std::size_t e=0;e<edges.size();++e)if(!(used>>e&1)){auto[u,w]=edges[e];if(u==v&&self(self,w,used|(1u<<e),root))return true;if(!directed&&w==v&&u!=v&&self(self,u,used|(1u<<e),root))return true;}return false;};if(start>=0)return search(search,start,0,start);for(int root=0;root<n;++root)if(search(search,root,0,root))return true;return false;
}
template<bool Directed>std::uint64_t test(){
 std::mt19937_64 random(20261031+Directed);std::uint64_t checks=0;
 for(int n=0;n<=7;++n)for(int rep=0;rep<350;++rep){EulerianTrail<Directed> graph(n);Edges edges;const int m=n?int(random()%9):0;for(int e=0;e<m;++e){int u=random()%n,v=random()%n;assert(graph.add_edge(u,v)==edges.size());edges.emplace_back(u,v);}for(int start=-1;start<n;++start)for(bool circuit:{false,true}){const bool expected=enumerate(Directed,n,edges,start,circuit);auto trail=graph.find(start,circuit);assert(bool(trail)==expected);if(trail){const auto&v=trail->vertices;const auto&e=trail->edges;assert(e.size()==edges.size());assert(v.size()==(n?edges.size()+1:0));if(n&&start!=-1)assert(v.front()==start);if(circuit)assert(trail->circuit());std::vector<int>count(edges.size());for(std::size_t i=0;i<e.size();++i){assert(e[i]<edges.size());++count[e[i]];auto[a,b]=edges[e[i]];assert((a==v[i]&&b==v[i+1])||(!Directed&&b==v[i]&&a==v[i+1]));}for(int c:count)assert(c==1);}++checks;}
  auto copy=graph;copy.reset();assert(copy.size()==0);auto moved=std::move(graph);assert(graph.size()==0);graph=moved;assert(graph.edge_count()==edges.size());graph=graph;
 }
#ifndef SKIP_RUNTIME_CHECKS
 {EulerianTrail<Directed> graph(2);bool caught=false;try{graph.find(-2);}catch(const std::out_of_range&){caught=true;}assert(caught);caught=false;try{graph.add_edge(0,2);}catch(const std::out_of_range&){caught=true;}assert(caught);}
#endif
 {int n=300000;EulerianTrail<Directed> graph(n);graph.reserve_edges(n);for(int v=1;v<n;++v)graph.add_edge(v-1,v);auto path=graph.find(0);assert(path&&path->vertices.front()==0&&path->vertices.back()==n-1&&path->edges.size()==std::size_t(n-1));assert(!graph.find(0,true));graph.add_edge(n-1,0);auto cycle=graph.find(0,true);assert(cycle&&cycle->circuit()&&cycle->edges.size()==std::size_t(n));++checks;}
 return checks;
}
int main(){std::cout<<"Eulerian checks="<<test<false>()+test<true>()<<'\n';}
