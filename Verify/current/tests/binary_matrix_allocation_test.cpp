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

#ifndef SIRKIT_BINARY_MATRIX_SOURCE
#define SIRKIT_BINARY_MATRIX_SOURCE "../../../Math/Binary_Matrix.cpp"
#endif
#include SIRKIT_BINARY_MATRIX_SOURCE
#include <iostream>
void run_checks(){
 std::size_t failures=0;
 for(int operation=0;operation<10;++operation){bool success=false;for(std::ptrdiff_t budget=0;budget<140;++budget){
  auto target=BinaryMatrix::identity(65);for(std::size_t i=0;i<64;++i)target.set(i,i+1);auto source=BinaryMatrix::identity(66);const auto old=target;const auto* ptr=target.row_words(0).data();std::vector<std::uint8_t>rhs(65,1);const auto before=live_allocations.load();allocation_countdown=budget;bool failed=false;
  try{
   if(operation==0){auto copy=target;assert(copy==old);}
   if(operation==1)target=source;
   if(operation==2)target.reset(70,70);
   if(operation==3){auto value=target.transpose();assert(value.rows()==65);}
   if(operation==4){auto value=target*target;assert(value.rows()==65);}
   if(operation==5)target*=target;
   if(operation==6)target.rank();
   if(operation==7)target.inverse();
   if(operation==8)target.solve(rhs);
   if(operation==9)target.pow(13);
  }catch(const std::bad_alloc&){failed=true;++failures;}
  allocation_countdown=-1;
  if(failed){assert(target==old&&target.row_words(0).data()==ptr);assert(live_allocations.load()==before);}else{success=true;break;}
 }assert(success);}
 {bool success=false;for(std::ptrdiff_t budget=0;budget<40;++budget){BinaryMatrix a(2,70);a.set(0,0);a.set(1,1);const auto old=a;const auto* ptr=a.row_words(0).data();std::vector<std::uint8_t>rhs{1,0};const auto before=live_allocations.load();allocation_countdown=budget;bool failed=false;try{a.solve(rhs);}catch(const std::bad_alloc&){failed=true;++failures;}allocation_countdown=-1;if(failed){assert(a==old&&a.row_words(0).data()==ptr);assert(live_allocations.load()==before);}else{success=true;break;}}assert(success);}
 {BinaryMatrix a(2,70),b(1,3);a.set(0,0);a.set(1,1);b.set(0,0);const auto source=a.solve(std::vector<std::uint8_t>{0,1});auto target=b.solve(std::vector<std::uint8_t>{1});assert(source&&target);const auto old_p=target->particular;const auto old_piv=target->pivot_columns;const auto old_k=target->kernel;const auto* ptr=target->kernel.row_words(0).data();bool success=false;
  for(std::ptrdiff_t budget=0;budget<40;++budget){const auto before=live_allocations.load();allocation_countdown=budget;bool failed=false;try{*target=*source;}catch(const std::bad_alloc&){failed=true;++failures;}allocation_countdown=-1;if(failed){assert(target->particular==old_p&&target->pivot_columns==old_piv&&target->kernel==old_k&&target->kernel.row_words(0).data()==ptr);assert(live_allocations.load()==before);}else{success=true;break;}}assert(success);allocation_countdown=0;auto moved=std::move(*target);*target=std::move(moved);target->kernel.clear();target->particular.clear();target->pivot_columns.clear();allocation_countdown=-1;}
 {auto a=BinaryMatrix::identity(65),b=BinaryMatrix::identity(65);std::vector<std::uint8_t>zero_rhs(3);BinaryMatrix zero(3,0);allocation_countdown=0;a.set(0,2).flip(1,2);a.swap_rows(0,1);a.xor_rows(1,0,63);a+=b;a^=a;auto moved=std::move(a);a=std::move(moved);a.clear();auto answer=zero.solve(zero_rhs);allocation_countdown=-1;assert(answer&&answer->particular.empty());}
 assert(failures>0);std::cout<<"binary matrix injected failures="<<failures<<'\n';
}
int main(){const auto before=live_allocations.load();run_checks();assert(live_allocations.load()==before);std::cout<<"live allocation delta="<<live_allocations.load()-before<<'\n';}
