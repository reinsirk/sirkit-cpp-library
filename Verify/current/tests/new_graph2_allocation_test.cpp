// SPDX-License-Identifier: CC0-1.0
#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <new>
namespace { std::atomic<std::ptrdiff_t> budget{-1};std::atomic<std::size_t> live{0}; }
void* operator new(std::size_t bytes){if(budget==0)throw std::bad_alloc();if(budget>0)--budget;if(void*p=std::malloc(bytes?bytes:1)){++live;return p;}throw std::bad_alloc();}
void operator delete(void*p)noexcept{if(p){--live;std::free(p);}}
void operator delete(void*p,std::size_t)noexcept{::operator delete(p);}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete[](void*p)noexcept{::operator delete(p);}
void operator delete[](void*p,std::size_t)noexcept{::operator delete(p);}
#ifndef LOW_LINK_SOURCE
#define LOW_LINK_SOURCE "../../../Graph/Low_Link.cpp"
#endif
#ifndef EULERIAN_SOURCE
#define EULERIAN_SOURCE "../../../Graph/Eulerian_Trail.cpp"
#endif
#ifndef MIN_COST_FLOW_SOURCE
#define MIN_COST_FLOW_SOURCE "../../../Graph/Min_Cost_Flow.cpp"
#endif
#include LOW_LINK_SOURCE
#include EULERIAN_SOURCE
#include MIN_COST_FLOW_SOURCE
#include <cassert>
#include <iostream>
int main(){
 const auto initial=live.load();std::size_t failures=0,successes=0;
 for(int b=0;b<110;++b){
  for(bool blocks:{false,true}){
   UndirectedLowLink g(5);g.add_edge(0,1);g.add_edge(1,2);g.add_edge(2,0);g.add_edge(2,3);g.build(true);auto old=g.bridges();auto points=g.articulation_points();const auto before=live.load();budget=b;
   try{g.build(blocks);budget=-1;++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(g.bridges()==old&&g.articulation_points()==points&&g.component_count()==2);assert(live==before);}
  }
  {
   UndirectedEulerianTrail g(5);for(int i=0;i<5;++i){g.add_edge(i,(i+1)%5);g.add_edge(i,i);}const auto before=live.load();budget=b;
   try{auto path=g.find();budget=-1;assert(path&&path->edges.size()==10);++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(g.edge_count()==10);assert(live==before);}assert(g.find());
  }
  for(bool slopes:{false,true}){
   MinCostFlow<> g(2);g.add_edge(0,1,2,-3);g.add_edge(0,1,3,0);g.add_edge(0,1,4,5);budget=b;
   try{if(slopes){auto result=g.slope(0,1);budget=-1;assert(result.back()==std::make_pair(std::int64_t{9},std::int64_t{14}));}else{auto result=g.flow(0,1);budget=-1;assert(result==std::make_pair(std::int64_t{9},std::int64_t{14}));}++successes;}catch(const std::bad_alloc&){budget=-1;++failures;}
   std::int64_t flow=0,cost=0;for(auto e:g.edges()){assert(e.flow>=0&&e.flow<=e.capacity);flow+=e.flow;cost+=e.flow*e.cost;}assert(flow>=0&&flow<=9);const auto expected=flow<=2?-3*flow:flow<=5?-6:-6+(flow-5)*5;assert(cost==expected);auto[f,c]=g.flow(0,1);assert(flow+f==9&&cost+c==14);
  }
  for(bool self:{false,true}){
   MinCostFlow<> g(3);budget=b;
   try{g.add_edge(0,self?0:2,2,3);budget=-1;++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(g.edge_count()==0);}g.add_edge(0,2,1,1);auto[f,c]=g.flow(0,2);assert(f==(self?1:(g.edge_count()==2?3:1)));assert(c==(self?1:(g.edge_count()==2?7:1)));
  }
  {
   MinCostFlow<> old(2),other(6);old.add_edge(0,1,2,3);old.flow(0,1);other.add_edge(0,5,7,-3);const auto before=live.load();budget=b;
   try{old=other;budget=-1;++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(old.size()==2&&old.get_edge(0).flow==2);assert(live==before);}
  }
  assert(live==initial);
 }
 assert(failures>0&&successes>0);std::cout<<"graph2 allocation failures="<<failures<<" successes="<<successes<<" live_delta="<<(live.load()-initial)<<'\n';
}
