// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef WAVELET_SOURCE
#define WAVELET_SOURCE "../../../DataStructure/Wavelet_Matrix.cpp"
#endif
#include WAVELET_SOURCE
#include <cassert>
#include <iostream>
#include <random>
template<class T> void run(std::mt19937_64& rng) {
    using U=std::make_unsigned_t<T>;
    constexpr auto digits=std::numeric_limits<U>::digits;
    auto restore=[](U value){return std::bit_cast<T>(static_cast<U>(value^(U{1}<<(digits-1))));};
    for(unsigned width=0;width<=digits;++width){
        const U mask=width==digits?std::numeric_limits<U>::max():static_cast<U>((U{1}<<width)-1);
        for(std::size_t n:{2,63,64,65,127,128,129,191,192,193,255,256,257}){
            std::vector<T> values(n);for(auto& v:values)v=restore(static_cast<U>(rng())&mask);
            values[0]=restore(0);values[1]=restore(mask);
            const WaveletMatrix<T> matrix(values);
            for(std::size_t i=0;i<n;++i)assert(matrix.access(i)==values[i]);
            for(unsigned q=0;q<40;++q){
                auto l=rng()%(n+1),r=rng()%(n+1);if(l>r)std::swap(l,r);
                const T x=q%2?values[rng()%n]:restore(static_cast<U>(rng())&mask);
                std::size_t less=0,equal=0;std::optional<T> pred,succ;
                for(auto i=l;i<r;++i){const T v=values[i];less+=v<x;equal+=v==x;
                    if(v<x&&(!pred||*pred<v))pred=v;
                    if(v>=x&&(!succ||v<*succ))succ=v;}
                assert(matrix.count_less(l,r,x)==less&&matrix.count(l,r,x)==equal);
                assert(matrix.predecessor(l,r,x)==pred&&matrix.successor(l,r,x)==succ);
                if(l<r){std::vector<T> sorted(values.begin()+l,values.begin()+r);std::sort(sorted.begin(),sorted.end());
                    const auto k=rng()%sorted.size();assert(matrix.kth_smallest(l,r,k)==sorted[k]);
                    assert(matrix.range_min(l,r)==sorted.front()&&matrix.range_max(l,r)==sorted.back());}
            }
        }
    }
}
int main(){
    if constexpr (sizeof(std::size_t) > sizeof(std::uint32_t)) {
        const std::size_t near_limit = std::numeric_limits<std::uint32_t>::max();
        ::sirkit::detail::CheckWaveletMatrixSize(near_limit);
        bool rejected = false;
        try { ::sirkit::detail::CheckWaveletMatrixSize(near_limit + 1); }
        catch (const std::length_error&) { rejected = true; }
        assert(rejected);
    }
#ifndef SKIP_RUNTIME_CHECKS
    const WaveletMatrix<int> matrix(std::vector<int>{4, -1, 0});
    assert(matrix.kth_largest(0, 3, 0) == 4 && matrix.kth_largest(0, 3, 2) == -1);
    const auto bad_rank = [](auto query) {
        try { query(); }
        catch (const std::out_of_range&) { return true; }
        return false;
    };
    assert(bad_rank([&] { (void)matrix.kth_largest(2, 1, 0); }));
    assert(bad_rank([&] { (void)matrix.kth_largest(0, 3, 3); }));
#endif
    std::mt19937_64 rng(0x571b174);run<std::int8_t>(rng);run<std::int16_t>(rng);run<std::int32_t>(rng);run<std::int64_t>(rng);std::cout<<"all B=0..64, signed extremes, word/AVX threshold boundaries OK\n";
}
