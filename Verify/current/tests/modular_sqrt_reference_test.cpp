// SPDX-License-Identifier: CC0-1.0
#ifndef MODULAR_SOURCE
#define MODULAR_SOURCE "../../../Math/Modular_Arithmetic.cpp"
#endif
#include MODULAR_SOURCE
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
using Big=boost::multiprecision::cpp_int;
static std::uint64_t checks=0;
static void check(bool okay){++checks;if(!okay)throw std::runtime_error("independent sqrt mismatch");}
static std::uint64_t reference_power(std::uint64_t a,std::uint64_t e,std::uint64_t p){Big r=Big{1}%p,b=Big{a}%p;for(;e;e>>=1,b=b*b%p)if(e&1)r=r*b%p;return r.convert_to<std::uint64_t>();}
static void verify(std::uint64_t a,std::uint64_t p){
 const auto actual=ModSqrt(a,p);const bool exists=a%p==0||reference_power(a,(p-1)/2,p)==1;
 check(actual.has_value()==exists);
 if(actual){check(Big{*actual}*(*actual)%p==Big{a}%p);check(*actual<=p-*actual);}
}
int main(){
 std::mt19937_64 rng(826319);
 const std::vector<std::uint64_t> primes{UINT64_C(2),UINT64_C(3),UINT64_C(5),UINT64_C(7),UINT64_C(17),UINT64_C(97),UINT64_C(257),UINT64_C(12289),UINT64_C(65521),UINT64_C(65537),UINT64_C(114689),UINT64_C(998244353),UINT64_C(1000000007),UINT64_C(4294967311),UINT64_C(2305843009213693951),UINT64_C(18446744069414584321),UINT64_C(18446744073709551557),UINT64_C(8549856358635032333),UINT64_C(15601257143829080093),UINT64_C(2419053326583747041),UINT64_C(10023533413112471713),UINT64_C(9852180570939523073),UINT64_C(4622514844609085441),UINT64_C(8340330347178754049),UINT64_C(12502041722571718657),UINT64_C(9400986153068265473),UINT64_C(4858761713006149633),UINT64_C(10612029933968424961),UINT64_C(4764829296478912513),UINT64_C(10346175718953582593),UINT64_C(11655034360658132993),UINT64_C(7273313398203351041),UINT64_C(3274116929098350593),UINT64_C(17798225727368200193),UINT64_C(16933534598913064961)};
 for(auto p:primes){
  for(auto a:std::array<std::uint64_t,8>{0,1,2,3,p-1,p,p+1,UINT64_MAX})verify(a,p);
  for(int i=0;i<100;++i){auto a=rng();if(i%2==0)a=(Big{a}*a%p).convert_to<std::uint64_t>();verify(a,p);}
 }
 for(std::uint64_t p=2;p<=1000;++p){bool prime=true;for(std::uint64_t d=2;d*d<=p;++d)if(p%d==0)prime=false;if(!prime)continue;for(std::uint64_t a=0;a<p;++a){std::uint64_t expected=p;for(std::uint64_t x=0;x<p;++x)if(x*x%p==a){expected=x;break;}const auto actual=ModSqrt(a,p);check(actual.has_value()==(expected!=p));if(actual)check(*actual==expected);}}
 bool threw=false;try{(void)ModSqrt(1,0);}catch(const std::invalid_argument&){threw=true;}check(threw);
 std::cout<<"PASS sqrt primes="<<primes.size()<<" independent Euler/exhaustive checks="<<checks<<'\n';
}
