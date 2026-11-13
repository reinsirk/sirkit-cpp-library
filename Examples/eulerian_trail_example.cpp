// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Eulerian_Trail.cpp"
#include <cassert>
#include <iostream>
int main(){
 UndirectedEulerianTrail graph(3);graph.add_edge(0,1);graph.add_edge(0,1);graph.add_edge(0,2);
 auto trail=graph.find(2);assert(trail&&trail->edges.size()==3&&!trail->circuit());for(int v:trail->vertices)std::cout<<v<<' ';std::cout<<'\n';assert(!graph.find(-1,true));
 DirectedEulerianTrail cycle(2);cycle.add_edge(0,1);cycle.add_edge(1,0);cycle.add_edge(0,0);auto answer=cycle.find(0,true);assert(answer&&answer->circuit());
}
