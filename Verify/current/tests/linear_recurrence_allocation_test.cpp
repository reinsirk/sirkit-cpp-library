// SPDX-License-Identifier: CC0-1.0
#ifndef HEADER
#define HEADER "../../../Math/Linear_Recurrence.cpp"
#endif
#include HEADER
#include <cassert>
#include <atomic>
#include <cstdlib>
#include <new>
#include <cstdint>
#include <iostream>
#include <vector>
static volatile std::uint64_t allocation_sink=0;
static std::atomic<std::int64_t> allowance{-1},live_blocks{0};
static void request(){if(allowance==0)throw std::bad_alloc();if(allowance>0)--allowance;}
[[gnu::noinline]] void*operator new(std::size_t n){request();void*p=std::malloc(n?n:1);if(!p)throw std::bad_alloc();++live_blocks;return p;}
[[gnu::noinline]] void*operator new[](std::size_t n){return ::operator new(n);}
[[gnu::noinline]] void operator delete(void*p)noexcept{if(p){--live_blocks;std::free(p);}}
[[gnu::noinline]] void operator delete[](void*p)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete(void*p,std::size_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete[](void*p,std::size_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void*operator new(std::size_t n,std::align_val_t a){request();void*p=nullptr;if(posix_memalign(&p,static_cast<std::size_t>(a),n?n:1))throw std::bad_alloc();++live_blocks;return p;}
[[gnu::noinline]] void*operator new[](std::size_t n,std::align_val_t a){return ::operator new(n,a);}
[[gnu::noinline]] void operator delete(void*p,std::align_val_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete[](void*p,std::align_val_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete(void*p,std::size_t,std::align_val_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete[](void*p,std::size_t,std::align_val_t)noexcept{::operator delete(p);}
int main(){
 const auto initial=live_blocks.load();
 for(int budget=0;budget<200;++budget){
  {std::vector<std::uint32_t>a(128),c(128),s(100);for(std::size_t i=0;i<a.size();++i){a[i]=static_cast<std::uint32_t>(i*17+1);c[i]=static_cast<std::uint32_t>(i+2);}for(std::size_t i=0;i<s.size();++i)s[i]=static_cast<std::uint32_t>(i*i+7);const auto before=live_blocks.load();
   allowance=budget;try{auto value=LinearRecurrenceTerm(a,c,UINT64_C(123456789123));allocation_sink=value;}catch(const std::bad_alloc&){}allowance=-1;assert(live_blocks.load()==before);
   allowance=budget;try{auto value=BerlekampMassey(s);std::uint64_t digest=value.size();for(auto x:value)digest=digest*UINT64_C(11995408973635179863)+x;allocation_sink=digest;}catch(const std::bad_alloc&){}allowance=-1;assert(live_blocks.load()==before);
   allowance=budget;try{auto value=LinearRecurrenceWeights(c,UINT64_MAX);std::uint64_t digest=value.size();for(auto x:value)digest=digest*UINT64_C(11995408973635179863)+x;allocation_sink=digest;}catch(const std::bad_alloc&){}allowance=-1;assert(live_blocks.load()==before);
   allowance=budget;try{auto value=LinearRecurrencePrefix(a,c,200);std::uint64_t digest=value.size();for(auto x:value)digest=digest*UINT64_C(11995408973635179863)+x;allocation_sink=digest;}catch(const std::bad_alloc&){}allowance=-1;assert(live_blocks.load()==before);
  }assert(live_blocks.load()==initial);
 }
 std::cout<<"PASS 800 allocation-failure budgets, zero outstanding allocation blocks\n";
}
