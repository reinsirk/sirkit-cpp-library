#ifndef MODULAR_SOURCE
#define MODULAR_SOURCE "ready_Modular_Arithmetic.cpp"
#endif
#include MODULAR_SOURCE
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <iostream>
#include <random>
#include <stdexcept>
using Big=boost::multiprecision::cpp_int;
static std::uint64_t checks=0;
static void require(bool okay){++checks;if(!okay)throw std::runtime_error("Montgomery independent mismatch");}
[[maybe_unused]] static Big inverse(Big a,const Big&m){Big b=m,u0=1,u1=0;while(b!=0){Big q=a/b,r=a-q*b,next=u0-q*u1;a=b;b=r;u0=u1;u1=next;}u0%=m;if(u0<0)u0+=m;return u0;}
static std::uint64_t power_ref(std::uint64_t a,std::uint64_t e,std::uint64_t m){Big r=Big{1}%m,b=Big{a}%m;for(;e;e>>=1,b=b*b%m)if(e&1)r=r*b%m;return r.convert_to<std::uint64_t>();}
int main(){
 std::mt19937_64 random(710263);std::uint64_t carry_cases=0,borrow_cases=0;
#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__)) && !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
 const Big R=Big{1}<<64;
 for(int i=0;i<20000;++i){
  auto m=random()|1;if(i%4==0)m=UINT64_MAX-(random()&UINT64_C(0x1fffe));if(m==1)m=3;
  auto a=random()%m,b=random()%m;if(i<64){m=UINT64_MAX;a=m-1;b=m-1;}else if(i<128){m=UINT64_MAX;a=1;b=1;}
  reinsirk::detail::ModularMontgomery64 arithmetic(m);
  const auto ri=inverse(R,Big{m});const auto expected=(Big{a}*b*ri%m).convert_to<std::uint64_t>();
  require(arithmetic.multiply(a,b)==expected);
  require(arithmetic.decode(arithmetic.encode(a))==a);
  const auto expected_product=(Big{a}*b%m).convert_to<std::uint64_t>();
  require(arithmetic.decode(arithmetic.multiply(arithmetic.encode(a),arithmetic.encode(b)))==expected_product);
  const Big ni=inverse(Big{m},R),product=Big{a}*b;Big q=(-product*ni)%R;if(q<0)q+=R;
  if(product<(product*ni%R)*m)++borrow_cases;
  if(Big{a}*b+q*m>=R*R)++carry_cases;
 }
 require(carry_cases>=64&&borrow_cases>=64);
#endif
 const std::array<std::uint64_t,18> exponents{0,1,2,3,63,127,128,129,32767,32768,32769,65535,65536,(std::uint64_t{1}<<31)-1,std::uint64_t{1}<<31,(std::uint64_t{1}<<32)-1,std::uint64_t{1}<<32,UINT64_MAX};
 for(int i=0;i<20000;++i){
  auto m=random()|1;if(i%4==0)m=UINT64_MAX-(random()&UINT64_C(0x1fffe));if(i%5==0)m=(std::uint64_t{1}<<32)+1;if(m==1)m=3;
  const auto a=random(),e=i%2?random():exponents[static_cast<std::size_t>(i/2)%exponents.size()];
  const auto expected=power_ref(a,e,m);require(ModPow(a,e,m)==expected);
#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__)) && !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
  reinsirk::detail::ModularMontgomery64 arithmetic(m);
  require(arithmetic.power<true>(a%m,e)==expected);require(arithmetic.power<false>(a%m,e)==expected);
#endif
 }
 for(const auto m:std::array<std::uint64_t,9>{1,2,UINT32_MAX,std::uint64_t{1}<<32,(std::uint64_t{1}<<32)+1,std::uint64_t{1}<<63,(std::uint64_t{1}<<63)+1,UINT64_MAX-1,UINT64_MAX})
  for(const auto a:std::array<std::uint64_t,5>{0,1,UINT32_MAX,UINT64_MAX-1,UINT64_MAX})for(auto e:exponents)require(ModPow(a,e,m)==power_ref(a,e,m));
 std::cout<<"PASS Montgomery checks="<<checks<<"; 20000 Big REDC, 20000 odd composite/prime powers, carry129="<<carry_cases<<" borrow="<<borrow_cases<<'\n';
}
