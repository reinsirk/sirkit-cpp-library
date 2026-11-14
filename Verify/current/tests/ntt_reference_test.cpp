// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef NTT_HEADER
#define NTT_HEADER "../../../Math/NTT.cpp"
#endif
#include NTT_HEADER
#include NTT_HEADER
#include <cstdio>
#include <cstdlib>
#include <random>
namespace {
std::uint64_t checks=0;void check(bool value){++checks;if(!value)std::abort();}
template<class Exception,class Function>void rejects(Function f){bool yes=false;try{f();}catch(const Exception&){yes=true;}check(yes);}
template<std::uint32_t Mod,class T>std::uint32_t norm(T v){if constexpr(std::is_signed_v<T>){auto r=static_cast<std::int64_t>(v)%static_cast<std::int64_t>(Mod);return static_cast<std::uint32_t>(r<0?r+Mod:r);}else return static_cast<std::uint32_t>(static_cast<std::uint64_t>(v)%Mod);}
template<std::uint32_t Mod,class T,class U>std::vector<std::uint32_t> reference(const std::vector<T>&a,const std::vector<U>&b){std::vector<std::uint32_t>result(a.empty()||b.empty()?0:a.size()+b.size()-1);for(std::size_t i=0;i<a.size();++i)for(std::size_t j=0;j<b.size();++j)result[i+j]=static_cast<std::uint32_t>((result[i+j]+std::uint64_t{norm<Mod>(static_cast<T>(a[i]))}*norm<Mod>(static_cast<U>(b[j])))%Mod);return result;}
std::uint32_t power(std::uint32_t a,std::uint32_t n,std::uint32_t mod){std::uint64_t result=1;while(n--!=0)result=result*a%mod;return static_cast<std::uint32_t>(result);}
template<std::uint32_t Mod,std::uint32_t Root>void test_mod(std::mt19937_64&rng){
 NTT<Mod,Root> engine;
 for(std::size_t n=1;n<=128&&n<=engine.max_length();n*=2){std::vector<std::uint32_t>a(n);for(auto&v:a)v=static_cast<std::uint32_t>(rng()%Mod);const auto saved=a;const auto omega=::sirkit::detail::ntt_power(Root,static_cast<std::uint32_t>((Mod-1)/n),Mod);std::vector<std::uint32_t>expected(n);for(std::size_t k=0;k<n;++k){const auto step=power(omega,static_cast<std::uint32_t>(k),Mod);std::uint64_t weight=1;for(std::size_t j=0;j<n;++j){expected[k]=static_cast<std::uint32_t>((expected[k]+std::uint64_t{a[j]}*weight)%Mod);weight=weight*step%Mod;}}engine.transform(a);check(a==expected);engine.transform(a,true);check(a==saved);}
 for(int rep=0;rep<180;++rep){const std::size_t n=rng()%145,m=rng()%145;std::vector<std::int64_t>a(n);std::vector<std::uint64_t>b(m);for(auto&v:a)v=static_cast<std::int64_t>(rng());for(auto&v:b)v=rng();if(n&&m&&std::min(n,m)>32&&std::bit_ceil(n+m-1)>engine.max_length()){rejects<std::length_error>([&]{engine.convolution(a,b);});continue;}const auto old_a=a;const auto old_b=b;const auto expected=reference<Mod>(a,b);check(engine.convolution(a,b)==expected);check(a==old_a&&b==old_b);std::vector<std::uint32_t>x(n),y(m);for(std::size_t i=0;i<n;++i)x[i]=norm<Mod>(a[i]);for(std::size_t i=0;i<m;++i)y[i]=norm<Mod>(b[i]);check(engine.convolution_normalized(x,y)==expected);check(engine.convolution_normalized(std::move(x),std::move(y))==expected);if(n<=32||std::bit_ceil(n?2*n-1:0)<=engine.max_length()){const auto self=reference<Mod>(a,a);check(engine.convolution(a,a)==self);check(engine.square(a)==self);}}
 const std::vector<std::int64_t> extremes{INT64_MIN,INT64_MAX,-1,0,1};check(engine.convolution(extremes,extremes)==reference<Mod>(extremes,extremes));check(engine.convolution(std::vector<bool>{true,false,true},std::vector<int>{1,-1})==reference<Mod>(std::vector<int>{1,0,1},std::vector<int>{1,-1}));check((ConvolutionMod<Mod,Root>(std::vector<int>{2,3},std::vector<int>{4,5})==reference<Mod>(std::vector<int>{2,3},std::vector<int>{4,5})));
 std::vector<std::uint32_t> values{1,2};engine.prepare(2);auto copied=engine;auto moved=std::move(engine);moved.transform(values);moved.transform(values,true);check(values==std::vector<std::uint32_t>({1,2}));engine.transform(values);engine.transform(values,true);check(values==std::vector<std::uint32_t>({1,2}));copied.clear_cache();check(copied.roots_capacity()==0);std::vector<std::uint32_t>empty;engine.transform(empty);check(empty.empty());rejects<std::length_error>([&]{engine.prepare(3);});rejects<std::length_error>([&]{engine.prepare(std::numeric_limits<std::size_t>::max());});
#ifndef SKIP_RUNTIME_CHECKS
 for(std::size_t n:{1,2}){std::vector<std::uint32_t>bad(n);bad.back()=Mod;rejects<std::invalid_argument>([&]{engine.transform(bad);});bad.assign(n,0);bad.back()=Mod;rejects<std::invalid_argument>([&]{engine.transform(bad,true);});}rejects<std::invalid_argument>([&]{engine.convolution_normalized({Mod},{1});});rejects<std::invalid_argument>([&]{engine.convolution_normalized({1},{Mod});});if(engine.max_length()>=128){rejects<std::invalid_argument>([&]{engine.convolution_normalized(std::vector<std::uint32_t>(33,Mod),std::vector<std::uint32_t>(33,1));});}
#endif
}
}
int main(){std::mt19937_64 rng(0x31c13cafe2026ULL);test_mod<998244353,3>(rng);test_mod<167772161,3>(rng);test_mod<469762049,3>(rng);test_mod<754974721,11>(rng);test_mod<17,3>(rng);test_mod<3,2>(rng);NTT<>engine;for(int n:{31,32,33,63,64,65,127,128,129,257,513,1024}){std::vector<std::uint32_t>a(n),b(n+1);for(auto&x:a)x=static_cast<std::uint32_t>(rng()%998244353);for(auto&x:b)x=static_cast<std::uint32_t>(rng()%998244353);check(engine.convolution(a,b)==reference<998244353>(a,b));check(engine.square(a)==reference<998244353>(a,a));}std::printf("NTT reference checks=%ju\n",static_cast<std::uintmax_t>(checks));}
