// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <new>
namespace {
std::atomic<std::ptrdiff_t> budget{-1};std::atomic<std::size_t> owned{0};
}
void* operator new(std::size_t bytes){if(budget==0)throw std::bad_alloc();if(budget>0)--budget;if(void*p=std::malloc(bytes?bytes:1)){++owned;return p;}throw std::bad_alloc();}
void operator delete(void*p)noexcept{if(p){--owned;std::free(p);}}
void operator delete(void*p,std::size_t)noexcept{::operator delete(p);}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete[](void*p)noexcept{::operator delete(p);}
void operator delete[](void*p,std::size_t)noexcept{::operator delete(p);}
#ifndef SCC_SOURCE
#define SCC_SOURCE "../../../Graph/SCC.cpp"
#endif
#ifndef TWO_SAT_SOURCE
#define TWO_SAT_SOURCE "../../../Graph/Two_SAT.cpp"
#endif
#ifndef HLD_SOURCE
#define HLD_SOURCE "../../../Graph/Heavy_Light_Decomposition.cpp"
#endif
#ifndef MAX_FLOW_SOURCE
#define MAX_FLOW_SOURCE "../../../Graph/Max_Flow.cpp"
#endif
#include SCC_SOURCE
#include TWO_SAT_SOURCE
#include HLD_SOURCE
#include MAX_FLOW_SOURCE
#include <cassert>
#include <iostream>
#include <string>
int main(){
 const auto initial=owned.load();std::size_t failures=0,successes=0;
 for(int b=0;b<80;++b){
  {
   scc_graph graph(8);graph.add_edge(0,1);graph.add_edge(1,0);graph.build();auto ids=graph.ids();const auto before=owned.load();
   budget=b;try{graph.build();budget=-1;++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(graph.ids()==ids);assert(owned==before);}assert(graph.size()==8&&graph.edge_count()==2);
  }
  {
   scc_graph graph(8);graph.add_edge(0,1);graph.add_edge(1,0);const auto expected=graph.scc();const auto before=owned.load();
   budget=b;try{auto groups=graph.scc();budget=-1;assert(groups==expected);++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(owned==before);assert(graph.component_count()==7);}
   assert(graph.size()==8&&graph.edge_count()==2);
  }
  {
   scc_graph graph(8);graph.add_edge(0,1);graph.add_edge(1,0);for(int i=0;i<100;++i)graph.add_edge(1,2);graph.add_edge(3,4);graph.build();
   const auto expected=graph.condensation(),multi=graph.condensation(false);const auto ids=graph.ids();const auto before=owned.load();
   budget=b;try{auto result=graph.condensation();auto all=graph.condensation(false);budget=-1;assert(result==expected&&all==multi);++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(owned==before);}
   assert(graph.ids()==ids&&graph.edge_count()==103);
  }
  {
   scc_graph graph(8);const auto expected=graph.scc();const auto before=owned.load();budget=b;
   try{auto groups=graph.scc();budget=-1;assert(groups==expected);++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(owned==before);}
   assert(graph.component_count()==8&&graph.edge_count()==0);
  }
  {
   two_sat sat(3);sat.force(0,true);sat.force(1,false);assert(sat.satisfiable());auto answer=sat.answer();const auto before=owned.load();budget=b;
   try{sat.satisfiable();budget=-1;++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(sat.answer()==answer);assert(owned==before);}assert(sat.clause_count()==2);
  }
  for(bool exclusive:{false,true}){
   two_sat sat(3);sat.force(0,true);sat.force(1,false);assert(sat.satisfiable());const auto before=owned.load();budget=b;
   try{if(exclusive)sat.add_xor(1,2);else sat.add_clause(1,false,2,true);budget=-1;++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(sat.clause_count()==2);assert(sat.value(0));assert(!sat.value(1));assert(owned==before);}
  }
  for(bool inverse:{false,true}){
   Graph graph(8);for(int v=1;v<8;++v){graph[0].push_back(v);graph[v].push_back(0);}HeavyLightDecomposition h(graph);
   const std::vector<std::string> values(8,std::string(1000,'x'));const auto ordered=h.reorder(values);const auto before=owned.load();budget=b;
   try{auto result=inverse?h.restore(ordered):h.reorder(values);budget=-1;assert(result==values);++successes;}
   catch(const std::bad_alloc&){budget=-1;++failures;assert(owned==before);assert(h.size()==8);assert(values==ordered);}
  }
  {
   Graph small{{1},{0}},large(40);for(int v=1;v<40;++v){large[v-1].push_back(v);large[v].push_back(v-1);}HeavyLightDecomposition h(small);const auto before=owned.load();budget=b;
   try{h.build(large,39);budget=-1;++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(h.size()==2&&h.root()==0&&h.jump(0,1,1)==1);assert(owned==before);}
  }
  for(bool self:{false,true}){
   mf_graph<> graph(4);graph.add_edge(0,1,7);graph.add_edge(1,3,7);assert(graph.flow(0,3)==7);const auto before=owned.load();budget=b;
   try{graph.add_edge(0,self?0:3,5);budget=-1;++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(graph.edge_count()==2);assert(graph.get_edge(0).flow==7&&graph.get_edge(1).flow==7);assert(owned==before);}
  }
  {
   mf_graph<> graph(4);graph.add_edge(0,1,7);graph.add_edge(1,3,7);const auto before=owned.load();budget=b;
   try{auto total=graph.flow(0,3);budget=-1;assert(total==7);++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(graph.get_edge(0).flow==0&&graph.get_edge(1).flow==0);assert(owned==before);}
  }
  {
   scc_graph a(3),other(100);other.add_edge(0,1);other.build();a.build();const auto before=owned.load();budget=b;
   try{a=other;budget=-1;++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(a.size()==3&&a.edge_count()==0&&a.component_count()==3);assert(owned==before);}
  }
  {
   two_sat a(3),other(100);a.force(0,true);a.satisfiable();other.force(0,false);other.satisfiable();const auto before=owned.load();budget=b;
   try{a=other;budget=-1;++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(a.size()==3&&a.value(0));assert(owned==before);}
  }
  {
   HeavyLightDecomposition a(Graph{{1},{0}}),other(Graph{{1,2},{0},{0}});const auto before=owned.load();budget=b;
   try{a=other;budget=-1;++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(a.size()==2&&a.root()==0&&a.jump(0,1,1)==1);assert(owned==before);}
  }
  {
   mf_graph<> a(2),other(8);a.add_edge(0,1,5);a.flow(0,1);for(int i=1;i<8;++i)other.add_edge(i-1,i,3);const auto before=owned.load();budget=b;
   try{a=other;budget=-1;++successes;}catch(const std::bad_alloc&){budget=-1;++failures;assert(a.size()==2&&a.edge_count()==1&&a.get_edge(0).flow==5);assert(owned==before);}
  }
  assert(owned==initial);
 }
 assert(failures>0&&successes>0);std::cout<<"graph allocation failures="<<failures<<" successes="<<successes<<" live_delta="<<(owned.load()-initial)<<'\n';
}
