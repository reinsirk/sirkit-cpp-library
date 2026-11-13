#ifndef HEADER
#define HEADER "baseline_C.hpp"
#endif
#include HEADER
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <random>
#include <vector>
struct Input{std::uint64_t a,e,m;};
[[gnu::noinline]] std::uint64_t operation(const Input& input,unsigned kind){
    asm volatile("":::"memory");
    if(!kind)return ModPow(input.a,input.e,input.m);
    const auto root=ModSqrt(input.a,input.m);return root?*root:UINT64_MAX;
}
std::uint64_t dataset_square(std::uint64_t a,std::uint64_t m){
    a%=m;std::uint64_t b=a,r=0;while(b){if(b&1)r=r>=m-a?r-(m-a):r+a;b>>=1;if(b)a=a>=m-a?a-(m-a):a+a;}return r;
}
int main(int argc,char**argv){
    if(argc!=4)return 2;
    const unsigned kind=static_cast<unsigned>(std::strtoul(argv[1],nullptr,10)),shape=static_cast<unsigned>(std::strtoul(argv[2],nullptr,10)),pattern=static_cast<unsigned>(std::strtoul(argv[3],nullptr,10));
    constexpr std::array<std::uint64_t,6> moduli{1000000007,(std::uint64_t{1}<<32)+1,UINT64_C(18446744073709551557),UINT64_MAX,UINT64_MAX-1,(std::uint64_t{1}<<63)+1};
    constexpr std::array<std::uint64_t,4> primes{1000000007,UINT64_C(18446744069414584321),UINT64_C(18446744073709551557),UINT64_C(2305843009213693951)};
    constexpr std::array<std::uint64_t,14> exponents{0,1,2,127,128,32767,32768,65535,(std::uint64_t{1}<<31)-1,std::uint64_t{1}<<31,(std::uint64_t{1}<<63)-1,UINT64_MAX,0,std::uint64_t{1}<<63};
    std::vector<Input> inputs(2048);std::mt19937_64 random(710263);unsigned at=0;
    for(auto& input:inputs){input.a=random();input.e=pattern==12?random():exponents[pattern];input.m=kind?primes[shape]:(shape==6?(random()|(std::uint64_t{1}<<63)):(shape==7?(random()|1):moduli[shape]));if(kind&&((at++)&1))input.a=dataset_square(input.a,input.m);}
    std::uint64_t checksum=0;timespec before{},after{};clock_gettime(CLOCK_THREAD_CPUTIME_ID,&before);const auto start=std::chrono::steady_clock::now();
    constexpr unsigned repeats=8;
    for(unsigned repeat=0;repeat<repeats;++repeat)for(const auto& input:inputs)checksum=checksum*UINT64_C(11995408973635179863)+operation(input,kind);
    asm volatile("":::"memory");const auto stop=std::chrono::steady_clock::now();clock_gettime(CLOCK_THREAD_CPUTIME_ID,&after);
    std::printf("%u,%u,%u,%u,%.6f,%.6f,%ju\n",kind,shape,pattern,unsigned(inputs.size()*repeats),std::chrono::duration<double,std::micro>(stop-start).count(),(after.tv_sec-before.tv_sec)*1e6+(after.tv_nsec-before.tv_nsec)/1000.0,static_cast<std::uintmax_t>(checksum));
}
