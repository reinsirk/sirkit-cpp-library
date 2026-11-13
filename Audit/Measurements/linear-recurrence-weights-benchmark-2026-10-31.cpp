// SPDX-License-Identifier: CC0-1.0
#include HEADER
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <vector>
[[gnu::noinline]] std::vector<std::uint32_t> query_weights(const std::vector<std::uint32_t>& c,std::uint64_t k){asm volatile("":::"memory");return LinearRecurrenceWeights(c,k);}
int main(int argc,char**argv){
 if(argc!=4)return 2;const auto d=static_cast<std::size_t>(std::strtoull(argv[1],nullptr,10));const auto k=static_cast<std::uint64_t>(std::strtoull(argv[2],nullptr,10));const auto q=static_cast<unsigned>(std::strtoul(argv[3],nullptr,10));std::vector<std::uint32_t> c(d);std::uint64_t random=3190183;for(auto& x:c){random^=random<<13;random^=random>>7;random^=random<<17;x=static_cast<std::uint32_t>(random);}std::uint64_t checksum=0;timespec before{},after{};clock_gettime(CLOCK_THREAD_CPUTIME_ID,&before);const auto start=std::chrono::steady_clock::now();
 for(unsigned i=0;i<q;++i){const auto result=query_weights(c,k);for(auto x:result)checksum=checksum*UINT64_C(11995408973635179863)+x;asm volatile("":::"memory");}
 const auto wall=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-start).count();clock_gettime(CLOCK_THREAD_CPUTIME_ID,&after);const auto cpu=(after.tv_sec-before.tv_sec)*INT64_C(1000000000)+after.tv_nsec-before.tv_nsec;std::printf("{\"cpu_ns\":%ju,\"wall_ns\":%ju,\"checksum\":%ju}\n",static_cast<std::uintmax_t>(cpu),static_cast<std::uintmax_t>(wall),static_cast<std::uintmax_t>(checksum));
}
