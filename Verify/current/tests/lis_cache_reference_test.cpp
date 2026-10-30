// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_LIS_SOURCE
#define SIRKIT_LIS_SOURCE "../../../Algorithm/LIS.cpp"
#endif
#include SIRKIT_LIS_SOURCE
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <new>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
static std::ptrdiff_t allocation_budget=-1;
static std::size_t live_blocks=0;
[[gnu::noinline]]void*operator new(std::size_t n){if(allocation_budget==0)throw std::bad_alloc();if(allocation_budget>0)--allocation_budget;if(void*p=std::malloc(n?n:1)){++live_blocks;return p;}throw std::bad_alloc();}
[[gnu::noinline]]void*operator new[](std::size_t n){return ::operator new(n);}
[[gnu::noinline]]void*operator new(std::size_t n,const std::nothrow_t&)noexcept{try{return ::operator new(n);}catch(...){return nullptr;}}
[[gnu::noinline]]void*operator new[](std::size_t n,const std::nothrow_t&)noexcept{try{return ::operator new[](n);}catch(...){return nullptr;}}
[[gnu::noinline]]static void release(void*p)noexcept{if(p)--live_blocks;std::free(p);}
void operator delete(void*p)noexcept{release(p);}void operator delete[](void*p)noexcept{release(p);}void operator delete(void*p,std::size_t)noexcept{release(p);}void operator delete[](void*p,std::size_t)noexcept{release(p);}
void operator delete(void*p,const std::nothrow_t&)noexcept{release(p);}void operator delete[](void*p,const std::nothrow_t&)noexcept{release(p);}
static std::uint64_t checks=0,cases=0,allocation_failures=0,nan_rejections=0;
static void require_impl(bool x,int line){++checks;if(!x){std::fprintf(stderr,"line %d: case=%ju checks=%ju\n",line,static_cast<std::uintmax_t>(cases),static_cast<std::uintmax_t>(checks));std::abort();}}
#define require(x) require_impl(bool(x),__LINE__)
using Logical=std::int64_t;
static std::uint64_t random_step(std::uint64_t&x){x^=x<<13;x^=x>>7;x^=x<<17;return x;}
template<class T>static T encode(Logical x){if constexpr(std::is_unsigned_v<T>){const T middle=T{1}<<(std::numeric_limits<T>::digits-1);return x<0?middle-static_cast<T>(-x):middle+static_cast<T>(x);}else return static_cast<T>(x);}
static std::vector<Logical> sample(bool repeated){std::vector<Logical>v;v.reserve(256);for(int i=0;i<256;++i)v.push_back(repeated?2-(i/2)%3:(i/8)*8+7-i%8);return v;}
template<bool Strict>static std::vector<Logical> tails_for_construction(const std::vector<Logical>&v){std::vector<Logical>tails;for(auto x:v){auto at=Strict?std::lower_bound(tails.begin(),tails.end(),x):std::upper_bound(tails.begin(),tails.end(),x);if(at==tails.end())tails.push_back(x);else*at=x;}return tails;}
template<bool Strict>static std::size_t length_for_construction(const std::vector<Logical>&v){return tails_for_construction<Strict>(v).size();}
// The test reference uses coordinate compression and Fenwick maximum DP, not LIS tails.
template<bool Strict,class T>static std::size_t reference_length(const std::vector<T>&v){std::vector<T>keys(v);std::sort(keys.begin(),keys.end());keys.erase(std::unique(keys.begin(),keys.end(),[](const T&a,const T&b){return !(a<b)&&!(b<a);}),keys.end());std::vector<std::size_t>bit(keys.size()+1);std::size_t best=0;
 for(const auto&x:v){const auto rank=std::size_t(std::lower_bound(keys.begin(),keys.end(),x)-keys.begin())+1;std::size_t got=0;for(auto p=Strict?rank-1:rank;p;p-=p&(~p+1))got=std::max(got,bit[p]);++got;best=std::max(best,got);for(auto p=rank;p<bit.size();p+=p&(~p+1))bit[p]=std::max(bit[p],got);}return best;}
template<bool Strict,class T>static void validate_indices(const std::vector<T>&v,const std::vector<std::size_t>&ix,std::size_t best){require(ix.size()==best);for(std::size_t j=0;j<ix.size();++j){require(ix[j]<v.size());if(j){require(ix[j-1]<ix[j]);require(Strict?v[ix[j-1]]<v[ix[j]]:!(v[ix[j]]<v[ix[j-1]]));}}}
template<bool Strict,class T>static void validate_sequence(const std::vector<T>&v,const std::vector<T>&seq,std::size_t best){require(seq.size()==best);std::size_t at=0;for(std::size_t j=0;j<seq.size();++j){if(j)require(Strict?seq[j-1]<seq[j]:!(seq[j]<seq[j-1]));while(at<v.size()&&(v[at]<seq[j]||seq[j]<v[at]))++at;require(at<v.size());++at;}}
template<bool Strict,class T>static void run(const std::vector<T>&v){++cases;const auto best=reference_length<Strict>(v);const auto before=v;validate_indices<Strict>(v,LISIndices<Strict>(v),best);validate_sequence<Strict>(v,LISSequence<Strict>(v),best);require(v==before);}
template<class T>static std::vector<T> numeric_input(std::size_t n,bool repeated){const auto first=sample(repeated);std::vector<T>v;v.reserve(n);for(auto x:first)v.push_back(encode<T>(x));std::uint64_t rng=923519;for(std::size_t i=v.size();i<n;++i)v.push_back(encode<T>(repeated?2-Logical((i/2)%3):Logical(random_step(rng)%1024)-512));if constexpr(std::is_floating_point_v<T>){v[256]=-std::numeric_limits<T>::infinity();v[257]=std::numeric_limits<T>::infinity();v[258]=-T{0};v[259]=T{0};}else{v[256]=std::numeric_limits<T>::lowest();v[257]=std::numeric_limits<T>::max();}return v;}
template<class T>static void numeric_cases(){for(auto n:{std::size_t{65535},std::size_t{65536},std::size_t{65537}})for(bool repeated:{false,true}){const auto v=numeric_input<T>(n,repeated);run<true>(v);run<false>(v);}}
template<class T>static std::vector<T> convert(const std::vector<Logical>&w){std::vector<T>v;v.reserve(w.size());for(auto x:w)v.push_back(encode<T>(x));return v;}
template<bool Strict,class T>static void count_boundaries(){for(std::size_t count:{31u,32u,33u,4095u,4096u,4097u})for(std::size_t prefix:{0u,20000u,200000u}){if((count<4095&&prefix!=0)||(count>=4095&&prefix==0))continue;auto w=sample(false);require(length_for_construction<Strict>(w)==32);std::uint64_t rng=519278;while(w.size()<prefix)w.push_back(Logical(random_step(rng)%256));auto length=length_for_construction<Strict>(w);const auto target=count+2;require(length<=target);Logical high=100000;while(length<target){w.push_back(high++);++length;}
 // Force the rightmost, then the three pivot-boundary intervals without extending.
 const auto old_tails=tails_for_construction<Strict>(w);require(old_tails.size()==target);
 const auto quarter=count/4;w.push_back(old_tails.back()-1);
 w.push_back(old_tails[1+3*quarter]);w.push_back(old_tails[1+2*quarter]);w.push_back(old_tails[1+quarter]);
 for(Logical q=127;q>=0;--q)w.push_back(q);const auto n=std::max<std::size_t>(65537,w.size()+512);Logical low=-1000;while(w.size()<n)w.push_back(low--);run<Strict>(convert<T>(w));}}
static std::vector<Logical> ratio_boundary(std::size_t processed){auto w=sample(false);auto len=length_for_construction<true>(w);Logical high=100000;while(len<4095){w.push_back(high++);++len;}Logical low=-1000;while(w.size()+1<processed)w.push_back(low--);w.push_back(high++);for(int j=0;j<64;++j)w.push_back(127-j);while(w.size()<65537)w.push_back(low--);return w;}
static std::vector<Logical> growing_after_cache(){auto w=sample(false);Logical high=100000;while(w.size()<65537)w.push_back(high++);return w;}
static std::vector<Logical> repeated_boundary(std::size_t run_length){auto w=sample(true);auto len=length_for_construction<false>(w);Logical high=100000;while(len<3841){w.push_back(high++);++len;}w.push_back(-1000);for(std::size_t i=0;i<run_length;++i)w.push_back(high++);Logical low=-1001;while(w.size()<65537)w.push_back(low--);return w;}
template<bool Strict,class T>static void allocation_case(const std::vector<T>&v){++cases;const auto best=reference_length<Strict>(v);const auto saved=v;const auto before=live_blocks;
 for(int operation=0;operation<2;++operation){bool success=false;for(std::ptrdiff_t allowed=0;allowed<128;++allowed){bool failed=false;allocation_budget=allowed;try{if(operation==0){const auto ix=LISIndices<Strict>(v);validate_indices<Strict>(v,ix,best);}else{const auto seq=LISSequence<Strict>(v);validate_sequence<Strict>(v,seq,best);}allocation_budget=-1;}catch(const std::bad_alloc&){allocation_budget=-1;failed=true;++allocation_failures;}require(live_blocks==before);require(v==saved);if(!failed){success=true;break;}}require(success);}}
template<class T>static void nan_cases(){
#ifndef SKIP_RUNTIME_CHECKS
 for(bool exit_cache:{false,true}){const auto raw=exit_cache?convert<T>(ratio_boundary(16383)):numeric_input<T>(65537,false);for(std::size_t pos:{std::size_t{0},std::size_t{255},std::size_t{256},std::size_t{65535},std::size_t{65536}}){auto v=raw;v[pos]=std::numeric_limits<T>::quiet_NaN();std::vector<unsigned char>saved(sizeof(T)*v.size());std::memcpy(saved.data(),v.data(),saved.size());
  for(int operation=0;operation<4;++operation){const auto before=live_blocks;bool threw=false;try{if(operation==0)(void)LISIndices<true>(v);if(operation==1)(void)LISIndices<false>(v);if(operation==2)(void)LISSequence<true>(v);if(operation==3)(void)LISSequence<false>(v);}catch(const std::invalid_argument&){threw=true;++nan_rejections;}require(threw);require(live_blocks==before);require(std::memcmp(saved.data(),v.data(),saved.size())==0);}
 }}
#endif
}
int main(){const auto before=live_blocks;require(length_for_construction<true>(sample(false))==32);
 numeric_cases<std::int32_t>();numeric_cases<std::int64_t>();numeric_cases<std::uint32_t>();numeric_cases<std::uint64_t>();numeric_cases<float>();numeric_cases<double>();
 run<true>(numeric_input<long double>(65537,false));run<false>(numeric_input<long double>(65537,false));
 count_boundaries<true,std::int32_t>();count_boundaries<false,std::int32_t>();count_boundaries<true,std::int64_t>();count_boundaries<false,std::int64_t>();
 for(auto processed:{16383u,16384u,16385u}){const auto v=convert<std::int64_t>(ratio_boundary(processed));run<true>(v);run<false>(v);}
 for(auto length:{255u,256u,257u})run<false>(convert<std::int64_t>(repeated_boundary(length)));
 allocation_case<true>(numeric_input<std::int64_t>(65537,false));allocation_case<false>(numeric_input<std::uint64_t>(65537,true));allocation_case<true>(convert<std::int64_t>(ratio_boundary(16383)));allocation_case<true>(convert<std::int32_t>(growing_after_cache()));allocation_case<false>(convert<double>(repeated_boundary(256)));
 nan_cases<float>();nan_cases<double>();nan_cases<long double>();require(live_blocks==before);
 std::printf("PASS cache reference cases=%ju checks=%ju allocation_failures=%ju NaN_rejections=%ju live_delta=%jd\n",static_cast<std::uintmax_t>(cases),static_cast<std::uintmax_t>(checks),static_cast<std::uintmax_t>(allocation_failures),static_cast<std::uintmax_t>(nan_rejections),static_cast<std::intmax_t>(live_blocks)-static_cast<std::intmax_t>(before));
}
