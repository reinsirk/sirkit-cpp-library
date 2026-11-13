// SPDX-License-Identifier: CC0-1.0
#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <new>
namespace {
std::atomic<std::ptrdiff_t> budget{-1};std::atomic<std::size_t> live{0};std::size_t checks=0,failures=0;
void check(bool value){++checks;if(!value)std::abort();}
}
[[gnu::noinline]]void* operator new(std::size_t n){if(budget==0)throw std::bad_alloc();if(budget>0)--budget;if(void*p=std::malloc(n?n:1)){++live;return p;}throw std::bad_alloc();}
[[gnu::noinline]]void operator delete(void*p)noexcept{if(p){--live;std::free(p);}}
[[gnu::noinline]]void operator delete(void*p,std::size_t)noexcept{::operator delete(p);}
[[gnu::noinline]]void* operator new[](std::size_t n){return ::operator new(n);}
[[gnu::noinline]]void operator delete[](void*p)noexcept{::operator delete(p);}
[[gnu::noinline]]void operator delete[](void*p,std::size_t)noexcept{::operator delete(p);}
#ifndef Z_HEADER
#define Z_HEADER "../../../String/Z_Algorithm.cpp"
#endif
#ifndef MANACHER_HEADER
#define MANACHER_HEADER "../../../String/Manacher.cpp"
#endif
#ifndef SUFFIX_HEADER
#define SUFFIX_HEADER "../../../String/Suffix_Array.cpp"
#endif
#ifndef NTT_HEADER
#define NTT_HEADER "../../../Math/NTT.cpp"
#endif
#include Z_HEADER
#include MANACHER_HEADER
#include SUFFIX_HEADER
#include NTT_HEADER
#include <cstdio>
#include <algorithm>
#include <numeric>
#include <string>
#include <string_view>
#include <limits>
#include <stdexcept>
struct LargeSignedText {
 int size()const{return std::numeric_limits<int>::max()-1;}
 int operator[](int)const{std::abort();}
};
int main(){const auto initial_live=live.load();{
 const std::string s(129,'a'),pattern(33,'a');const std::vector<std::int64_t>numbers(129,-7);const std::vector<std::uint32_t>a(129,1),b(97,1);const auto saved_a=a,saved_b=b;
 std::vector<std::int64_t> unique_numbers(129);std::vector<std::string> unique_words(129);
 for(int i=0;i<129;++i){const int value=128-i;unique_numbers[i]=std::int64_t{value}*1000000007-5000000000000;unique_words[i]=std::string{static_cast<char>('a'+value/26),static_cast<char>('a'+value%26)};}
 const auto saved_unique_numbers=unique_numbers;const auto saved_unique_words=unique_words;
 std::string patterned,distinct_lms;
 for(int i=0;i<257;++i)patterned.push_back("mississippi"[i%11]);
 for(char c='a';c<='z';++c){distinct_lms.push_back('{');distinct_lms.push_back(c);}
 const auto saved_patterned=patterned,saved_distinct_lms=distinct_lms;
 const auto reference=[](const std::string&text){std::vector<int>order(text.size());std::iota(order.begin(),order.end(),0);std::sort(order.begin(),order.end(),[&](int x,int y){return std::string_view(text).substr(x)<std::string_view(text).substr(y);});return order;};
 const auto expected_patterned=reference(patterned),expected_distinct_lms=reference(distinct_lms);
 for(int kind=0;kind<14;++kind)for(int allowance=0;allowance<80;++allowance){const auto owned=live.load();budget=allowance;try{
  if(kind==0){auto z=z_algorithm(s);budget=-1;check(z[0]==129&&z.back()==1);}
  if(kind==1){auto matches=ZMatches(pattern,s);budget=-1;check(matches.size()==97&&matches.front()==0&&matches.back()==96);}
  if(kind==2){PalindromeRadii p(s);budget=-1;check(p.count()==129*130/2);}
  if(kind==3){auto lengths=PalindromicLengths(s);budget=-1;check(lengths.size()==257&&lengths[128]==129);}
  if(kind==4){auto sa=suffix_array(s);budget=-1;check(sa.size()==129&&sa[0]==128&&sa.back()==0);}
  if(kind==5){auto sa=suffix_array(numbers);budget=-1;check(sa.size()==129&&sa[0]==128&&sa.back()==0);}
  if(kind==6){NTT<> e;auto c=e.convolution(a,b);budget=-1;check(c.size()==225&&c[0]==1&&c[96]==97&&c.back()==1);}
  if(kind==7){NTT<> e;auto c=e.square(a);budget=-1;check(c.size()==257&&c[128]==129&&c.back()==1);}
  if(kind==8){auto sa=suffix_array(s);auto lcp=lcp_array(s,sa);budget=-1;check(lcp.size()==128&&lcp[0]==1&&lcp.back()==128);}
  if(kind==9){auto c=ConvolutionMod(a,b);budget=-1;check(c.size()==225&&c[0]==1&&c[96]==97&&c.back()==1);}
  if(kind==10){auto sa=suffix_array(unique_numbers);budget=-1;check(sa.size()==129&&sa.front()==128&&sa.back()==0);}
  if(kind==11){auto sa=suffix_array(unique_words);budget=-1;check(sa.size()==129&&sa.front()==128&&sa.back()==0);}
  if(kind==12){auto sa=suffix_array(patterned);budget=-1;check(sa==expected_patterned);}
  if(kind==13){auto sa=suffix_array(distinct_lms);budget=-1;check(sa==expected_distinct_lms);}
 }catch(const std::bad_alloc&){budget=-1;++failures;}check(live.load()==owned);check(a==saved_a&&b==saved_b);check(unique_numbers==saved_unique_numbers&&unique_words==saved_unique_words);check(patterned==saved_patterned&&distinct_lms==saved_distinct_lms);}
 for(int allowance=0;allowance<4;++allowance){PalindromeRadii p(std::string("abba"));const auto old_odd=p.odd,old_even=p.even;budget=allowance;try{p.assign(s);budget=-1;check(p.size()==129);}catch(const std::bad_alloc&){budget=-1;++failures;check(p.odd==old_odd&&p.even==old_even);}}
 // size()がintでも、出力長の倍化を符号付き演算で行わない。
 budget=0;bool rejected=false;
 try{(void)PalindromicLengths(LargeSignedText{});}catch(const std::bad_alloc&){budget=-1;rejected=true;++failures;}catch(const std::length_error&){budget=-1;rejected=true;}
 budget=-1;check(rejected);
 NTT<>e;e.prepare(16);const auto old_roots=e.roots_capacity();budget=0;try{e.prepare(512);budget=-1;check(false);}catch(const std::bad_alloc&){budget=-1;++failures;check(e.roots_capacity()==old_roots);}auto c=e.convolution(a,b);check(c.size()==225&&c[96]==97);
}check(live.load()==initial_live);std::printf("allocation checks=%zu failures=%zu live_delta=%zu\n",checks,failures,live.load()-initial_live);}
