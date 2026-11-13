// SPDX-License-Identifier: CC0-1.0
#include "../Graph/SCC.cpp"
#include <cassert>
#include <iostream>
int main(){
 scc_graph graph(5);
 graph.add_edge(0,1);graph.add_edge(1,0);graph.add_edge(1,2);graph.add_edge(2,3);graph.add_edge(3,2);graph.add_edge(3,4);
 const auto groups=graph.scc();assert(groups.size()==3);assert(graph.id(0)==graph.id(1));assert(graph.id(1)<graph.id(2));
 for(const auto& group:groups){for(int v:group)std::cout<<v<<' ';std::cout<<'\n';}
 const Graph dag=graph.condensation();assert(dag.size()==3);
}
