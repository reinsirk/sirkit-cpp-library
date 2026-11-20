// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Min_Cost_Flow.cpp"
#include <cassert>
#include <iostream>

using namespace sirkit;
int main(){
 MinCostFlow<> network(2);network.add_edge(0,1,2,-3);auto expensive=network.add_edge(0,1,4,5);
 auto[first,cost]=network.flow(0,1,1);assert(first==1&&cost==-3);
 auto points=network.slope(0,1);assert((points.back()==std::pair<std::int64_t,std::int64_t>{5,17}));
 for(auto[flow,value]:points)std::cout<<flow<<' '<<value<<'\n';
 network.change_cost(expensive,4);network.reset_flow();auto result=network.flow(0,1);assert(result.first==6&&result.second==10);assert(!network.min_cut(0)[1]);
}
