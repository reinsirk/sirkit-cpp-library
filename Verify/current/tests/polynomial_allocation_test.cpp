// SPDX-License-Identifier: CC0-1.0
#ifndef HEADER
#define HEADER "../../../Math/Polynomial.cpp"
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
 FormalPowerSeries::clear_cache();const auto initial=live_blocks.load();
 for(int budget=0;budget<1200;++budget){
  {std::vector<std::uint32_t>points(129),coefficients(129),values(129);
   for(std::size_t i=0;i<points.size();++i){points[i]=static_cast<std::uint32_t>(i+10);coefficients[i]=static_cast<std::uint32_t>(i*7+2);values[i]=static_cast<std::uint32_t>(i*i+1);}
   ProductTree source(points);FormalPowerSeries::clear_cache();
   for(int operation=0;operation<5;++operation){const auto before=live_blocks.load();
    {ProductTree target(std::vector<int>{2,7,9});const auto original=target.points();bool failed=false;allowance=budget;
     try{std::uint64_t digest=0;
      if(operation==0){for(auto value:source.evaluate(coefficients))digest=digest*UINT64_C(11995408973635179863)+value;}
      if(operation==1){const auto interpolated=source.interpolate(values);for(auto value:interpolated.coefficients())digest=digest*UINT64_C(11995408973635179863)+value;}
      if(operation==2){const auto shifted=TaylorShift(coefficients,17);for(auto value:shifted.coefficients())digest=digest*UINT64_C(11995408973635179863)+value;}
      if(operation==3){target.assign(points);digest=target.size();}
      if(operation==4){target=source;digest=target.size();}
      allocation_sink=digest;
     }catch(const std::bad_alloc&){failed=true;}allowance=-1;
     if(failed&&(operation==3||operation==4)){assert(target.points()==original);const auto got=target.evaluate(std::vector<int>{1,2,1});for(std::size_t i=0;i<original.size();++i)assert(got[i]==(original[i]+1)*(original[i]+1));}
    }FormalPowerSeries::clear_cache();assert(live_blocks.load()==before);
   }
  }assert(live_blocks.load()==initial);
 }
 std::cout<<"PASS 6000 allocation failure budgets; strong assign/copy and zero outstanding allocations\n";
}
