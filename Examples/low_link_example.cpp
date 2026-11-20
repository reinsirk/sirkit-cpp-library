// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Low_Link.cpp"
#include <cassert>
#include <iostream>

using namespace sirkit;
int main(){
 UndirectedLowLink graph(5);graph.add_edge(0,1);graph.add_edge(1,2);graph.add_edge(2,0);auto bridge=graph.add_edge(2,3);graph.add_edge(3,4);graph.add_edge(3,4);
 graph.build();assert(graph.is_bridge(bridge));assert(graph.is_articulation(2)&&graph.is_articulation(3));assert(graph.two_edge_component(0)==graph.two_edge_component(2));assert(graph.two_edge_component(2)!=graph.two_edge_component(3));
 for(auto id:graph.bridges())std::cout<<id<<'\n';
 graph.build(true);auto edge_blocks=graph.biconnected_edge_components();assert(edge_blocks.size()==3);assert(graph.biconnected_vertex_components().size()==3);assert(graph.bridge_forest().size()==2);
}
