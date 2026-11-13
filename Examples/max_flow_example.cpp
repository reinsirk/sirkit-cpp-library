// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Max_Flow.cpp"
#include <cassert>
#include <iostream>
int main(){
 mf_graph<> network(4);auto id=network.add_edge(0,1,5);network.add_edge(0,2,2);network.add_edge(1,2,2);network.add_edge(1,3,3);network.add_edge(2,3,4);
 assert(network.flow(0,3,4)==4);assert(network.flow(0,3)==3);assert(network.flow(0,3)==0);
 const auto edge=network.get_edge(id);assert(edge.cap==5&&edge.flow==5);const auto cut=network.min_cut(0);assert(cut[0]&&!cut[3]);
 std::cout<<edge.flow<<'\n';auto copy=network;for(int i=0;i<static_cast<int>(copy.edge_count());++i){auto e=copy.get_edge(i);copy.change_edge(i,e.cap,0);}assert(copy.flow(0,3)==7);
}
