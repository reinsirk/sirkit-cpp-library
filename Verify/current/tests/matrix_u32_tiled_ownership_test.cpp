// SPDX-License-Identifier: CC0-1.0
#ifndef HEADER
#define HEADER "../../../Math/Matrix.cpp"
#endif
#include HEADER
#include HEADER
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>
static std::atomic<std::int64_t> allowance{-1},live_blocks{0};
static void request(){if(allowance==0)throw std::bad_alloc();if(allowance>0)--allowance;}
[[gnu::noinline]] void*operator new(std::size_t n){request();auto*p=std::malloc(n?n:1);if(!p)throw std::bad_alloc();++live_blocks;return p;}
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
struct alignas(64) Owned {
 static inline int live=0,copy_budget=-1,op_budget=-1;
 std::uint32_t value;
 Owned(std::uint32_t x):value(x){++live;}
 Owned(const Owned&x):value(x.value){if(copy_budget==0)throw std::runtime_error("copy");if(copy_budget>0)--copy_budget;++live;}
 Owned(Owned&&x)noexcept:value(x.value){++live;}
 Owned&operator=(const Owned&)=default;
 ~Owned(){--live;}
 Owned&operator+=(const Owned&x){if(op_budget==0)throw std::runtime_error("operation");if(op_budget>0)--op_budget;value+=x.value;return *this;}
 friend Owned operator*(const Owned&a,const Owned&b){return Owned(a.value*b.value);}
 friend bool operator==(const Owned&a,const Owned&b){return a.value==b.value;}
};
static volatile std::uint32_t sink=0;
int main(){const auto initial=live_blocks.load();std::uint64_t failures=0;
 {
  Matrix<std::uint32_t>a(5,129),b(129,4103);
  for(std::size_t i=0;i<a.size();++i)a.values()[i]=static_cast<std::uint32_t>(i*UINT64_C(174891232173));
  for(std::size_t i=0;i<b.size();++i)b.values()[i]=static_cast<std::uint32_t>(i*UINT64_C(4130482517));
  const auto saved_a=a,saved_b=b;const auto expected=a*b;const auto before=live_blocks.load();
  for(int budget=0;budget<3;++budget){
   allowance=budget;try{auto c=a*b;allowance=-1;assert(c==expected);sink=c(4,4102);}catch(const std::bad_alloc&){allowance=-1;++failures;}
   assert(a==saved_a&&b==saved_b&&live_blocks.load()==before);
   {auto target=a;const auto owned=live_blocks.load();allowance=budget;bool threw=false;try{target*=b;}catch(const std::bad_alloc&){threw=true;++failures;}allowance=-1;assert(target==(threw?saved_a:expected));assert(live_blocks.load()==owned);}
   assert(live_blocks.load()==before);
  }
  auto detached=a;detached(0,0)^=1;assert(a==saved_a);auto moved=std::move(detached);assert(detached.rows()==0&&detached.cols()==0&&detached.empty());detached.assign(1,1,7);assert(moved.rows()==5&&detached(0,0)==7);
 }
 assert(live_blocks.load()==initial);
 for(int budget=0;budget<80;++budget){
  {Matrix<Owned>a{{1,2,3},{4,5,6}},b{{7,8},{9,10},{11,12}};const auto saved_a=a,saved_b=b;const auto expected=a*b;const auto initial_live=Owned::live;const auto before=live_blocks.load();assert(reinterpret_cast<std::uintptr_t>(a.values().data())%64==0);
   Owned::copy_budget=budget;try{auto c=a*b;(void)c;}catch(const std::runtime_error&){++failures;}Owned::copy_budget=-1;assert(Owned::live==initial_live&&live_blocks.load()==before&&a==saved_a&&b==saved_b);
   {auto target=a;const auto owned=Owned::live;Owned::op_budget=budget;bool threw=false;try{target*=b;}catch(const std::runtime_error&){threw=true;++failures;}Owned::op_budget=-1;assert(target==(threw?saved_a:expected));assert(Owned::live==owned+(threw?0:static_cast<int>(expected.size())-static_cast<int>(a.size())));}
   assert(Owned::live==initial_live&&live_blocks.load()==before);
  }assert(Owned::live==0&&live_blocks.load()==initial);
 }
 bool overflow=false;try{Matrix<std::uint32_t>a(2,std::numeric_limits<std::size_t>::max());(void)a;}catch(const std::length_error&){overflow=true;}assert(overflow);
#ifndef SKIP_RUNTIME_CHECKS
 bool mismatch=false;try{(void)(Matrix<std::uint32_t>(1,2)*Matrix<std::uint32_t>(3,4));}catch(const std::invalid_argument&){mismatch=true;}assert(mismatch);
#endif
 assert(Owned::live==0&&live_blocks.load()==initial);std::printf("PASS matrix allocation/copy/operation failures=%ju; deep copy/move/align64; live_delta=0\n",static_cast<std::uintmax_t>(failures));
}
