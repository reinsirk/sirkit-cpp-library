// SPDX-License-Identifier: CC0-1.0
#ifndef HEADER
#define HEADER "../../../Math/Convolution.cpp"
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
 for(int budget=0;budget<150;++budget){
  {std::vector<std::uint32_t>a(129,UINT32_MAX-1),b(65,17);std::vector<std::int64_t>x(129,-17),y(65,19);const auto before=live_blocks.load();
   for(int operation=0;operation<5;++operation){allowance=budget;try{
    CRTConvolution c;
    std::uint64_t digest=0;
    if(operation==0){const auto values=c.modular(a,b,UINT32_MAX);for(auto value:values)digest=digest*UINT64_C(11995408973635179863)+value;}
    if(operation==1){const auto values=c.integer(x,y);for(auto value:values)digest=digest*UINT64_C(11995408973635179863)+static_cast<std::uint64_t>(value);}
    if(operation==2){const auto values=c.integer_square(x);for(auto value:values)digest=digest*UINT64_C(11995408973635179863)+static_cast<std::uint64_t>(value);}
    if(operation==3){c.prepare(1024);auto copy=c;c.clear_cache();digest=copy.roots_capacity();}
    if(operation==4){c.prepare(64);CRTConvolution other;other.prepare(1024);c=other;digest=c.roots_capacity();}
    allocation_sink=digest;
   }catch(const std::bad_alloc&){}allowance=-1;assert(live_blocks.load()==before);}
  }assert(live_blocks.load()==initial);
 }
 std::cout<<"PASS 750 allocation failure budgets, zero outstanding allocation blocks\n";
}
