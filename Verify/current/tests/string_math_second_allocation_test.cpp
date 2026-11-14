// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <new>
namespace{std::atomic<std::ptrdiff_t>budget{-1};std::atomic<std::size_t>live{0};std::size_t checks=0,failures=0;void check(bool x){++checks;if(!x)std::abort();}}
[[gnu::noinline]]void*operator new(std::size_t n){if(budget==0)throw std::bad_alloc();if(budget>0)--budget;if(void*p=std::malloc(n?n:1)){++live;return p;}throw std::bad_alloc();}
[[gnu::noinline]]void operator delete(void*p)noexcept{if(p){--live;std::free(p);}}
[[gnu::noinline]]void operator delete(void*p,std::size_t)noexcept{::operator delete(p);}
[[gnu::noinline]]void*operator new[](std::size_t n){return ::operator new(n);}
[[gnu::noinline]]void operator delete[](void*p)noexcept{::operator delete(p);}
[[gnu::noinline]]void operator delete[](void*p,std::size_t)noexcept{::operator delete(p);}
#ifndef POLLARD_HEADER
#define POLLARD_HEADER "../../../Math/Pollard_Rho.cpp"
#endif
#ifndef AHO_HEADER
#define AHO_HEADER "../../../String/Aho_Corasick.cpp"
#endif
#ifndef FPS_HEADER
#define FPS_HEADER "../../../Math/Formal_Power_Series.cpp"
#endif
#include POLLARD_HEADER
#include AHO_HEADER
#include FPS_HEADER
#include <cstdio>
#include <string>
int main(){FormalPowerSeries::clear_cache();const auto initial=live.load();{
 const std::string text="abacabadabacaba",long_word(200,'a');AhoCorasick<> original;original.add("a");original.add("aba");original.add("");original.build();const auto expected_counts=original.match_counts(text);
 for(int allowance=0;allowance<120;++allowance){AhoCorasick<>candidate;candidate.add("a");candidate.add("");budget=allowance;try{candidate.add(long_word);budget=-1;}catch(const std::bad_alloc&){budget=-1;++failures;}candidate.build();auto counts=candidate.match_counts(text);check(counts.size()==2||counts.size()==3);check(counts[0]==8&&counts[1]==text.size()+1);if(counts.size()==3)check(counts[2]==0);}
 for(int allowance=0;allowance<10;++allowance){auto candidate=original;candidate.clear();candidate.add("a");candidate.add("aba");candidate.add("");budget=allowance;try{candidate.build();budget=-1;}catch(const std::bad_alloc&){budget=-1;++failures;candidate.build();}check(candidate.match_counts(text)==expected_counts);}
 for(int allowance=0;allowance<20;++allowance){AhoCorasick<>candidate;candidate.add("b");candidate.build();budget=allowance;try{candidate=original;budget=-1;check(candidate.match_counts(text)==expected_counts);}catch(const std::bad_alloc&){budget=-1;++failures;check(candidate.count_total("bb")==2);}}
 using F=FormalPowerSeries;std::vector<std::uint32_t>coefficients(65);coefficients[0]=1;coefficients[1]=2;coefficients[2]=1;const F f(coefficients);const auto before=f;
 for(int kind=0;kind<7;++kind)for(int allowance=0;allowance<150;++allowance){F::clear_cache();const auto owned=live.load();budget=allowance;try{
 if(kind==0){auto value=f.inverse(65);budget=-1;check(value.size()==65);}
 if(kind==1){auto value=f.log(65);budget=-1;check(value.size()==65);}
 if(kind==2){auto value=f.derivative().exp(0);budget=-1;check(value.empty());}
 if(kind==3){auto value=f.pow(5,65);budget=-1;check(value.size()==65);}
 if(kind==4){auto value=f.sqrt(65);budget=-1;check(value&&value->size()==65);}
 if(kind==5){auto value=(f*f).divmod(f);budget=-1;check(value.first.size()==3&&value.second.empty());}
 if(kind==6){auto value=F{0,1}.exp(65);budget=-1;check(value.size()==65);}
 }catch(const std::bad_alloc&){budget=-1;++failures;}F::clear_cache();check(live.load()==owned);check(f==before);}
 for(int allowance=0;allowance<20;++allowance){const auto owned=live.load();budget=allowance;try{auto value=PrimeFactors64(1000000016000000063ULL);budget=-1;check(value==std::vector<std::uint64_t>({1000000007,1000000009}));}catch(const std::bad_alloc&){budget=-1;++failures;}check(live.load()==owned);}
}FormalPowerSeries::clear_cache();check(live.load()==initial);std::printf("second allocation checks=%zu failures=%zu live_delta=%zu\n",checks,failures,live.load()-initial);}
