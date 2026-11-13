#ifndef HEADER
#define HEADER "baseline_Mont.hpp"
#endif
#include HEADER
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
struct Input{std::uint64_t a,m,e;};
[[gnu::noinline]] std::uint64_t operation(const Input& input,unsigned kind){
    asm volatile("":::"memory");
    if(kind)return ModPow(input.a,input.e,input.m);
    const auto root=ModSqrt(input.a,input.m);return root?*root:UINT64_MAX;
}
int main(int argc,char**argv){
    if(argc!=3)return 2;const auto kind=static_cast<unsigned>(std::strtoul(argv[2],nullptr,10));
    FILE* file=std::fopen(argv[1],"rb");if(!file)return 3;
    std::vector<Input> inputs(1024);if(std::fread(inputs.data(),sizeof(Input),inputs.size(),file)!=inputs.size())return 4;std::fclose(file);
    std::uint64_t checksum=0;timespec before{},after{};clock_gettime(CLOCK_THREAD_CPUTIME_ID,&before);const auto start=std::chrono::steady_clock::now();
    constexpr unsigned repeats=8;
    for(unsigned repeat=0;repeat<repeats;++repeat)for(const auto& input:inputs)checksum=checksum*UINT64_C(11995408973635179863)+operation(input,kind);
    asm volatile("":::"memory");const auto stop=std::chrono::steady_clock::now();clock_gettime(CLOCK_THREAD_CPUTIME_ID,&after);
    std::printf("%u,%u,%.6f,%.6f,%ju\n",kind,unsigned(inputs.size()*repeats),std::chrono::duration<double,std::micro>(stop-start).count(),(after.tv_sec-before.tv_sec)*1e6+(after.tv_nsec-before.tv_nsec)/1000.0,static_cast<std::uintmax_t>(checksum));
}
