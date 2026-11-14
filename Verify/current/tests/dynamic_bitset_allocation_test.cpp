// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <new>
#include <stdexcept>
static std::ptrdiff_t allocation_countdown = -1;
static std::atomic<std::size_t> live_allocations{0};
static void allocation_check() {
    if (allocation_countdown == 0) throw std::bad_alloc();
    if (allocation_countdown > 0) --allocation_countdown;
}
void* operator new(std::size_t size) {
    allocation_check();
    if (void* p = std::malloc(size == 0 ? 1 : size)) { ++live_allocations; return p; }
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* p) noexcept { if (p) { --live_allocations; std::free(p); } }
void operator delete[](void* p) noexcept { ::operator delete(p); }
void operator delete(void* p, std::size_t) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { ::operator delete(p); }
void* operator new(std::size_t size, std::align_val_t alignment) {
    allocation_check();
    const auto align = static_cast<std::size_t>(alignment);
    const auto bytes = (size / align + 1) * align;
    if (void* p = std::aligned_alloc(align, bytes)) { ++live_allocations; return p; }
    throw std::bad_alloc();
}
void* operator new[](std::size_t size, std::align_val_t alignment) { return ::operator new(size, alignment); }
void operator delete(void* p, std::align_val_t) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::align_val_t) noexcept { ::operator delete(p); }
void operator delete(void* p, std::size_t, std::align_val_t) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::size_t, std::align_val_t) noexcept { ::operator delete(p); }

#ifndef SIRKIT_BITSET_SOURCE
#define SIRKIT_BITSET_SOURCE "../../../DataStructure/Dynamic_Bitset.cpp"
#endif
#include SIRKIT_BITSET_SOURCE
#include <iostream>
void run_checks(){
 std::size_t failures=0;
 for(int operation=0;operation<10;++operation){bool success=false;
  for(std::ptrdiff_t limit=0;limit<20;++limit){
   DynamicBitset target("1001010111100010101010101011110010001100010110001111001010010101011110001010101010101111001000110001011000111100101001010101111000101010101010111100100011000101100011110010100101010111100010101010101011110010001100010110001111001010010101011110001010101010101111001000110001011000111100101"), source(500,true);
   const auto old=target.to_string();const auto* ptr=target.words().data();const auto old_count=target.count();const auto before=live_allocations.load();allocation_countdown=limit;bool failed=false;
   try{
    if(operation==0)target.resize(10000,true);
    if(operation==1)target.reserve_bits(10000);
    if(operation==2)target=source;
    if(operation==3){auto copy=target;assert(copy.size()==target.size());}
    if(operation==4)target.to_string();
    if(operation==5){auto copy=~target;assert(copy.size()==target.size());}
    if(operation==6){auto copy=target<<1;assert(copy.size()==target.size());}
    if(operation==7){auto copy=target>>1;assert(copy.size()==target.size());}
    if(operation==8){auto copy=target|target;assert(copy.size()==target.size());}
    if(operation==9){DynamicBitset copy{std::string_view(old)};assert(copy.size()==target.size());}
   }catch(const std::bad_alloc&){failed=true;++failures;}
   allocation_countdown=-1;
   if(failed){assert(target.to_string()==old&&target.count()==old_count&&target.words().data()==ptr);assert(live_allocations.load()==before);}
   else{success=true;break;}
  }assert(success);
 }
 {DynamicBitset a(65,true),b(65);allocation_countdown=0;a.set(0).reset(64).flip(1);a.flip().set().reset();a|=b;a&=b;a^=b;a<<=1;a>>=64;a.shift_or(65);a.resize(64);a.resize(63,true);assert(a.count()==0);auto c=std::move(a);a=std::move(c);a.clear();allocation_countdown=-1;assert(a.empty()&&c.empty());}
#ifndef SKIP_RUNTIME_CHECKS
 {const auto before=live_allocations.load();bool rejected=false;try{DynamicBitset invalid("10000000000000000000000000000000000000000000000000000000000000000x");}catch(const std::invalid_argument&){rejected=true;}assert(rejected&&live_allocations.load()==before);}
#endif
 assert(failures>0);std::cout<<"dynamic bitset injected failures="<<failures<<'\n';
}
int main(){const auto before=live_allocations.load();run_checks();assert(live_allocations.load()==before);std::cout<<"live allocation delta="<<live_allocations.load()-before<<'\n';}
