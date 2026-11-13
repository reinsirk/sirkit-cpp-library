// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_RH_SOURCE
#define SIRKIT_RH_SOURCE "../../../String/RollingHash.cpp"
#endif
#include SIRKIT_RH_SOURCE
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <string>
namespace {
std::atomic<std::ptrdiff_t> budget{-1},live{0};std::size_t checks=0,failures=0;
void require(bool ok){++checks;if(!ok)std::abort();}
std::uint64_t mul(std::uint64_t a,std::uint64_t b){std::uint64_t r=0;while(b){if(b&1)r=(r+a)%RollingHash::modulus;a=(a+a)%RollingHash::modulus;b>>=1;}return r;}
void inspect(const RollingHash&h,std::string_view s){require(h.size()==s.size());std::uint64_t ref=0;for(std::size_t i=0;i<s.size();++i){ref=(mul(ref,h.base())+static_cast<unsigned char>(s[i]))%RollingHash::modulus;require(h.get(0,i+1)==ref);}require(h.whole()==ref);require(h.lcp(0,h,0)==s.size());}
}
[[gnu::noinline]] void* operator new(std::size_t n){if(budget==0)throw std::bad_alloc();if(budget>0)--budget;if(void*p=std::malloc(n?n:1)){++live;return p;}throw std::bad_alloc();}
[[gnu::noinline]] void* operator new[](std::size_t n){return ::operator new(n);}
[[gnu::noinline]] void release(void*p)noexcept{if(p)--live;std::free(p);}
void operator delete(void*p)noexcept{release(p);}void operator delete[](void*p)noexcept{release(p);}void operator delete(void*p,std::size_t)noexcept{release(p);}void operator delete[](void*p,std::size_t)noexcept{release(p);}
int main(){const auto owned=live.load();{std::string s(4097,'\0');for(std::size_t i=0;i<s.size();++i)s[i]=static_cast<char>((i*137)%256);const auto before=live.load();for(int at=0;at<3;++at){bool success=false;budget=at;try{RollingHash h(s,RollingHash::modulus-2);budget=0;inspect(h,s);budget=-1;success=true;}catch(const std::bad_alloc&){budget=-1;++failures;}require(success==(at==2));require(live==before);}
 RollingHash target("old state",911382323);const auto previous=target.whole();const auto count=live.load();for(int at=0;at<3;++at){bool success=false;budget=at;try{target.assign(s);budget=0;inspect(target,s);budget=-1;success=true;}catch(const std::bad_alloc&){budget=-1;++failures;require(target.whole()==previous);require(target.size()==9);require(live==count);}if(success)break;}
 target.clear();budget=0;target.assign(s);inspect(target,s);target.assign(s);inspect(target,s);target.assign(std::string_view(s).substr(0,4096));inspect(target,std::string_view(s).substr(0,4096));target.assign(s);inspect(target,s);budget=-1;}
 require(live==owned);std::printf("PASS first AVX allocation checks=%zu failure_points=%zu live_delta=%td avx2=%d\n",checks,failures,live.load()-owned,__builtin_cpu_supports("avx2")?1:0);}
