// SPDX-License-Identifier: CC0-1.0
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <new>
static volatile std::int64_t allocations_live=0,fail_after=-1;
static void allocation_check(){if(fail_after==0)throw std::bad_alloc();if(fail_after>0)--fail_after;}
[[gnu::noinline]] void* operator new(std::size_t bytes){allocation_check();if(void*p=std::malloc(bytes?bytes:1)){++allocations_live;return p;}throw std::bad_alloc();}
[[gnu::noinline]] void operator delete(void*p)noexcept{if(p){--allocations_live;std::free(p);}}
[[gnu::noinline]] void operator delete(void*p,std::size_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void* operator new[](std::size_t bytes){return ::operator new(bytes);}
[[gnu::noinline]] void operator delete[](void*p)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete[](void*p,std::size_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void* operator new(std::size_t bytes,std::align_val_t alignment){allocation_check();void*p=nullptr;if(posix_memalign(&p,static_cast<std::size_t>(alignment),bytes?bytes:1)==0){++allocations_live;return p;}throw std::bad_alloc();}
[[gnu::noinline]] void* operator new[](std::size_t bytes,std::align_val_t alignment){return ::operator new(bytes,alignment);}
[[gnu::noinline]] void operator delete(void*p,std::align_val_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete[](void*p,std::align_val_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete(void*p,std::size_t,std::align_val_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete[](void*p,std::size_t,std::align_val_t)noexcept{::operator delete(p);}
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>
#ifndef SLOPE_TRICK_SOURCE
#define SLOPE_TRICK_SOURCE "../../../DataStructure/Slope_Trick.cpp"
#endif
#include SLOPE_TRICK_SOURCE
using I=std::int64_t;
std::vector<I> snapshot(const SlopeTrick& f){std::vector<I>v;for(int x=-30;x<=30;++x)v.push_back(f.eval(x));return v;}
void failure_cases(){
 SlopeTrick source;source.add_abs(3);source.add_abs(-4);source.add_left_hinge(7);source.shift(-1,2);source.add_constant(-8);auto expected=snapshot(source);SlopeTrick other;for(int i=0;i<20;++i)other.add_abs(i-10);int failures=0;
 for(int operation=0;operation<10;++operation)for(int allowance=0;allowance<100;++allowance){SlopeTrick t=source;auto before=allocations_live;fail_after=allowance;bool failed=false;
  try{switch(operation){case 0:t.add_abs(30);break;case 1:t.add_abs(-30);break;case 2:t.add_left_hinge(30);break;case 3:t.add_right_hinge(-30);break;case 4:t.merge(other);break;case 5:t.merge(t);break;case 6:t=other;break;case 7:{SlopeTrick copy(t);assert(copy.size()==t.size());break;}case 8:t.reserve(1000,1000);break;case 9:t.add_abs(0);break;}}
  catch(const std::bad_alloc&){failed=true;++failures;}fail_after=-1;
  if(failed){assert(snapshot(t)==expected);assert(allocations_live>=before);}assert(snapshot(source)==expected);if(!failed)break;
 }
 assert(failures>15);std::printf("slope allocation failures=%d strong logical guarantee\n",failures);
 auto t=source;t.reserve(100,100);fail_after=0;t.add_abs(10);t.add_left_hinge(-10);t.add_right_hinge(20);t.shift(-2,3);t.add_constant(2);auto range=t.argmin();assert(range.lower&&range.upper);t.eval(2);fail_after=-1;
 const I mn=std::numeric_limits<I>::min(),mx=std::numeric_limits<I>::max();int overflows=0;
 for(int operation=0;operation<7;++operation){SlopeTrick f;f.add_abs(0);auto old=snapshot(f);bool failed=false;try{switch(operation){case 0:f.shift(mx);f.shift(1);break;case 1:f.add_constant(mx);f.add_constant(1);break;case 2:f.add_left_hinge(mn);f.shift(-1);f.add_left_hinge(mx);break;case 3:f.add_right_hinge(mx);f.shift(1);f.add_right_hinge(mn);break;case 4:f.add_abs(mn);break;case 5:f.add_constant(mx);f.merge(source);break;case 6:f.shift(mn);f.add_abs(mx);break;}}catch(const std::overflow_error&){failed=true;++overflows;}assert(failed);}
 assert(overflows==7);std::printf("slope overflow guards=%d\n",overflows);
}
int main(){assert(allocations_live==0);{failure_cases();}assert(allocations_live==0);std::printf("normal_live0=%" PRId64 "\n",allocations_live);}
