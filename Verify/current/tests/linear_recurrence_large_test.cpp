// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef HEADER
#define HEADER "../../../Math/Linear_Recurrence.cpp"
#endif
#include HEADER
#include <array>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
static std::uint32_t independent_power(std::uint32_t value,std::uint64_t exponent){std::uint64_t result=1;for(;exponent;exponent>>=1,value=static_cast<std::uint32_t>(std::uint64_t{value}*value%998244353))if(exponent&1)result=result*value%998244353;return static_cast<std::uint32_t>(result);}
static void require(bool condition){if(!condition)throw std::runtime_error("large recurrence mismatch");}
int main(){
 const std::uint64_t k=UINT64_MAX;
 for(const std::size_t d:std::array<std::size_t,11>{1,31,32,33,63,64,65,511,512,513,100003}){
  std::vector<std::uint32_t>a(d),c(d,1);c.back()=2;std::uint32_t value=1;for(auto&x:a){x=value;value=static_cast<std::uint32_t>(std::uint64_t{value}*2%998244353);}require(LinearRecurrenceTerm(a,c,k)==independent_power(2,k));require(LinearRecurrenceTerm(a,c,d-1)==a.back());
 }
 const std::size_t d=100003;std::mt19937_64 rng(1235);std::vector<std::uint32_t>a(d),c(d,0);for(auto&x:a)x=static_cast<std::uint32_t>(rng());c.back()=1;require(LinearRecurrenceTerm(a,c,k)==a[k%d]%998244353);
 c.back()=0;require(LinearRecurrenceTerm(a,c,k)==0);require(LinearRecurrenceTerm(a,c,d)==0);
 require(LinearRecurrenceTerm(std::vector<std::uint32_t>{},std::vector<std::uint32_t>{},k)==0);
 std::cout<<"PASS dense geometric/periodic order100003, transient zero recurrence, thresholds, uint64 maximum\n";
}
