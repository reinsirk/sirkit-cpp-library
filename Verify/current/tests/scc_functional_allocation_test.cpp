// SPDX-License-Identifier: CC0-1.0
#include <cstdlib>
#include <cstdio>
#include <new>
#include <cstddef>
static volatile std::ptrdiff_t budget=-1;static volatile std::size_t live=0;static std::size_t checks=0;
[[gnu::noinline]] void* operator new(std::size_t n){if(budget==0)throw std::bad_alloc();if(budget>0)budget=budget-1;if(void*p=std::malloc(n?n:1)){live=live+1;return p;}throw std::bad_alloc();}
[[gnu::noinline]] void* operator new[](std::size_t n){return ::operator new(n);}
[[gnu::noinline]] void operator delete(void*p)noexcept{if(p)live=live-1;std::free(p);}[[gnu::noinline]] void operator delete[](void*p)noexcept{::operator delete(p);}[[gnu::noinline]] void operator delete(void*p,std::size_t)noexcept{::operator delete(p);}[[gnu::noinline]] void operator delete[](void*p,std::size_t)noexcept{::operator delete(p);}
#ifndef SIRKIT_SCC_TEST_SOURCE
#define SIRKIT_SCC_TEST_SOURCE "../../../Graph/SCC.cpp"
#endif
#include SIRKIT_SCC_TEST_SOURCE
static void check_at(bool x,int line){++checks;if(!x){std::fprintf(stderr,"failed line=%d checks=%zu live=%zu budget=%td\n",line,checks,live,budget);std::abort();}}
#define check(x) check_at((x),__LINE__)
int main(){std::size_t failures=0,successes=0;const auto original_live=live;{
 for(int shape=0;shape<3;++shape){const int n=257;scc_graph graph(n);graph.reserve_edges(n);
  for(int v=0;v<n;++v){const int u=shape==2&&v==1?0:v;const int to=shape==0?(v+1)%n:v/2;graph.add_edge(u,to);}
  graph.build();const auto expected=graph.ids();const auto count=graph.component_count();
  for(int at=0;at<32;++at){const auto before=live;const auto* prior=graph.ids().data();bool success=false;
   budget=at;try{graph.build();budget=-1;success=true;}catch(const std::bad_alloc&){budget=-1;++failures;check(graph.ids().data()==prior);}
   check(graph.ids()==expected&&graph.component_count()==count&&graph.edge_count()==std::size_t(n));
   check(live==before);if(success){++successes;break;}
  }
 }
 }check(live==original_live&&failures>=12&&successes==3);std::printf("PASS SCC functional/near-functional allocation failures=%zu checks=%zu live_delta=%td\n",failures,checks,std::ptrdiff_t(live)-std::ptrdiff_t(original_live));}
