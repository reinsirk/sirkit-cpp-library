// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include "../../../Algorithm/LIS.cpp"
#include <cassert>
#include <random>
#include <cstdio>
#include <numeric>
template<bool Strict>void check(const std::vector<std::int64_t>&v){std::vector<std::int64_t>d;for(auto x:v){auto at=Strict?std::lower_bound(d.begin(),d.end(),x):std::upper_bound(d.begin(),d.end(),x);if(at==d.end())d.push_back(x);else*at=x;}auto ix=LISIndices<Strict>(v);auto seq=LISSequence<Strict>(v);assert(ix.size()==d.size()&&seq.size()==ix.size());for(std::size_t i=0;i<ix.size();++i){assert(ix[i]<v.size()&&seq[i]==v[ix[i]]);if(i)assert(ix[i-1]<ix[i]&&(Strict?v[ix[i-1]]<v[ix[i]]:v[ix[i-1]]<=v[ix[i]]));}}
int main(){
    const std::vector<std::int64_t> small{9, 1, 2, 0, 3};
    const std::vector<std::size_t> exact{1, 2, 4};
    assert((::sirkit::detail::LISIndicesByWidth<true, 4>(small) == exact));
    assert((::sirkit::detail::LISIndicesByWidth<true, 5>(small) == exact));
    // 先頭のサンプルに中間位置への置換がなく、探索型へ切り替えない入力。
    std::vector<std::int64_t> prefix_then_low(65536, -1);
    std::iota(prefix_then_low.begin(), prefix_then_low.begin()+32, std::int64_t{0});
    check<true>(prefix_then_low);
    assert(LISLength(prefix_then_low) == 32);
    std::mt19937_64 r(82841);unsigned count=1;for(std::size_t n:{std::size_t(65535),std::size_t(65536),std::size_t(65537),std::size_t(200000)})for(int dom:{2,3,4,8,16,256})for(int shape=0;shape<3;++shape){std::vector<std::int64_t>a(n);for(std::size_t i=0;i<n;++i)a[i]=shape==1&&i>n/3?std::int64_t(1000000+i):shape==2&&i>n/3?-std::int64_t(i):std::int64_t(r()%dom);check<true>(a);check<false>(a);count+=2;}std::printf("%u domain/cache exit cases passed\n",count);}
