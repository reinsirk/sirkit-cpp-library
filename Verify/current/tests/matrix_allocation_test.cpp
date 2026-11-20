// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef HEADER
#define HEADER "../../../Math/Gaussian_Elimination.cpp"
#endif
#include HEADER
#include "../common/matrix_field.hpp"
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <new>
#include <cstdint>
#include <iostream>
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
 for(int budget=0;budget<80;++budget){
  {Matrix<VerifyMatrixField>a(8,8);for(std::size_t i=0;i<8;++i)for(std::size_t j=0;j<8;++j)a(i,j)=VerifyMatrixField(i==j?2:1);std::vector<VerifyMatrixField>b(8,1);const auto before=live_blocks.load();
   allowance=budget;try{auto result=a*a;allocation_sink=result(0,0).get();}catch(const std::bad_alloc&){}allowance=-1;assert(live_blocks.load()==before);
   allowance=budget;try{auto result=MatrixPower(a,UINT64_C(12345));allocation_sink=result(0,0).get();}catch(const std::bad_alloc&){}allowance=-1;assert(live_blocks.load()==before);
   allowance=budget;try{auto result=MatrixInverse(a);allocation_sink=result?(*result)(0,0).get():0;}catch(const std::bad_alloc&){}allowance=-1;assert(live_blocks.load()==before);
   allowance=budget;try{auto result=SolveLinearSystem(a,b);allocation_sink=result?result->particular[0].get():0;}catch(const std::bad_alloc&){}allowance=-1;assert(live_blocks.load()==before);
   allowance=budget;try{auto result=GaussianElimination(a,true);allocation_sink=result.rank();}catch(const std::bad_alloc&){}allowance=-1;assert(live_blocks.load()==before);
  }assert(live_blocks.load()==initial);
 }
 std::cout<<"PASS 400 allocation-failure budgets; zero outstanding allocation blocks\n";
}
