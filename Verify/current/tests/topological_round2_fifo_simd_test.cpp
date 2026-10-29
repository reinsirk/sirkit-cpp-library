// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TEST_TOPOLOGICAL_SOURCE
#define SIRKIT_TEST_TOPOLOGICAL_SOURCE "../../../Graph/Topological.cpp"
#endif
#include SIRKIT_TEST_TOPOLOGICAL_SOURCE
#include <algorithm>
#include <cassert>
#include <iostream>
int main(){std::size_t cases=0;
#if !defined(SIRKIT_DISABLE_SIMD) && defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
 if(__builtin_cpu_supports("avx2")){
  for(std::size_t n:{std::size_t(4096),std::size_t(4097),std::size_t(4098),std::size_t(4099),std::size_t(4100),std::size_t(4101),std::size_t(4102),std::size_t(4103),std::size_t(8193)})for(unsigned mask=0;mask<256;++mask){std::vector<std::uint32_t>degrees(n);for(std::size_t i=0;i<n;++i)degrees[i]=(mask>>(i%8)&1)?0:(i%4==0?UINT32_MAX:i%4==1?UINT32_C(0x80000000):i%4==2?2:1);std::vector<int>expected;bool multiple=false;for(std::size_t i=0;i<n;++i){if(degrees[i]==0)expected.push_back(int(i));multiple|=degrees[i]>1;}auto before=degrees;std::vector<int>order;order.reserve(n);auto[count,bits]=reinsirk::detail::TopologicalInitialFifoAvx2(degrees.data(),order,n);assert(count==expected.size());assert((bits>1)==multiple);assert(order.size()==std::min(n,count+8));order.resize(count);assert(order==expected);assert(degrees==before);++cases;}
 }
#endif
 std::cout<<cases<<" FIFO SIMD masks/tails/high-degree cases passed\n";
}
