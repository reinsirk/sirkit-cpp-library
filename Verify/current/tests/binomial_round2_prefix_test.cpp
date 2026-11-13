// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_BINOMIAL_TEST_SOURCE
#define SIRKIT_BINOMIAL_TEST_SOURCE "../../../Math/Combination/Binomial_Mod.cpp"
#endif
#include SIRKIT_BINOMIAL_TEST_SOURCE
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
std::uint32_t exact(std::int64_t n,std::int64_t k,std::uint32_t modulus){if(n<0||k<0||k>n)return 0;k=std::min(k,n-k);boost::multiprecision::cpp_int x=1;for(std::int64_t i=1;i<=k;++i){x*=n-k+i;x/=i;}return static_cast<std::uint32_t>(x%modulus);}
int main(){std::mt19937_64 rng(825917);std::uint64_t checks=0;std::vector<std::uint32_t> mods{1,3,9,27,243,6561,531441,387420489,5,125,625,1220703125,72,360,1000000000,1000000006,998244353,1000000007,1000003,4294967291u,4294967295u};for(unsigned bit=1;bit<=31;++bit)mods.push_back(std::uint32_t{1}<<bit);
for(auto m:mods)for(std::uint32_t limit:{31u,64u,257u,4095u,4096u,4097u,65536u,65537u}){BinomialOptions options;options.table_limit=limit;options.max_multiplicative_steps=100000;BinomialMod b(m,0,options);for(std::uint32_t target:{0u,1u,2u,3u,7u,15u,16u,17u,31u,32u,33u,63u,64u,65u,127u,128u,129u,255u,256u,257u,4095u,4096u,4097u,65535u,65536u,65537u}){if(target>limit)continue;b.prepare(target);for(int i=0;i<96;++i){const auto n=std::int64_t(rng()%(std::uint64_t(target)+100));const auto k=std::int64_t(rng()%18);const auto want=exact(n,k,m);assert(b.COM(n,k)==want);assert(b.COM(n,n-k)==want);checks+=2;}auto c=b;assert(c.COM(77,11)==exact(77,11,m));++checks;}
for(std::int64_t n:{std::int64_t(0),std::int64_t(1),std::int64_t(limit),INT64_C(4294967296),INT64_MAX-100,INT64_MAX})for(std::int64_t k:{0,1,2,3,7,15}){assert(b.COM(n,k)==exact(n,k,m));++checks;}}
std::cout<<"PASS prefix/factorial incremental oracle checks="<<checks<<'\n';}
