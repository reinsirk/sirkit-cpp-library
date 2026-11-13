// SPDX-License-Identifier: CC0-1.0
#ifndef WAVELET_SOURCE
#define WAVELET_SOURCE "../../../DataStructure/Wavelet_Matrix.cpp"
#endif
#include WAVELET_SOURCE
#include <cassert>
#include <iostream>
#include <random>

static std::uint64_t checks = 0;
static void check(bool result) { ++checks; assert(result); }
template<class T>
void check_range(const WaveletMatrix<T>& matrix, const std::vector<T>& values,
                 std::size_t left, std::size_t right, const std::vector<T>& probes) {
    // 頻度・前後の値はソート/rank実装を使わず、元配列の直接走査で求める。
    for (const T x : probes) {
        std::size_t less = 0, equal = 0, less_equal = 0;
        std::optional<T> predecessor, successor;
        for (auto i = left; i < right; ++i) {
            const T v = values[i];
            less += v < x; equal += v == x; less_equal += v <= x;
            if (v < x && (!predecessor || *predecessor < v)) predecessor = v;
            if (v >= x && (!successor || v < *successor)) successor = v;
        }
        check(matrix.count_less(left, right, x) == less);
        check(matrix.count_less_equal(left, right, x) == less_equal);
        check(matrix.rank(x, left, right) == equal);
        check(matrix.count(left, right, x) == equal);
        check(matrix.predecessor(left, right, x) == predecessor);
        check(matrix.successor(left, right, x) == successor);
        for (const T y : probes) {
            std::size_t count = 0;
            for (auto i = left; i < right; ++i) count += x <= values[i] && values[i] < y;
            check(matrix.range_freq(left, right, x, y) == count);
        }
    }
    if (left != right) {
        auto copy = std::vector<T>(values.begin() + left, values.begin() + right);
        for (std::size_t k = 0; k < copy.size(); ++k) {
            std::nth_element(copy.begin(), copy.begin() + k, copy.end());
            check(matrix.kth_smallest(left, right, k) == copy[k]);
            check(matrix.kth_largest(left, right, right - left - 1 - k) == copy[k]);
        }
        check(matrix.range_min(left, right) == *std::min_element(copy.begin(), copy.end()));
        check(matrix.range_max(left, right) == *std::max_element(copy.begin(), copy.end()));
    }
}
template<class T> void exercise(std::mt19937_64& random) {
    const T low = std::numeric_limits<T>::lowest(), high = std::numeric_limits<T>::max();
    for (std::size_t n : {0,1,2,3,4,7,63,64,65,127,128,129,191,192,193}) {
        for (int mode = 0; mode < 5; ++mode) {
            std::vector<T> values(n);
            for (std::size_t i = 0; i < n; ++i) {
                if (mode == 0) values[i] = static_cast<T>(high - static_cast<T>(i % 4));
                if (mode == 1) values[i] = static_cast<T>(low + static_cast<T>(i % 4));
                if (mode == 2) values[i] = i % 2 ? high : low;
                if (mode == 3) values[i] = static_cast<T>(i % 5);
                if (mode == 4) values[i] = std::bit_cast<T>(static_cast<std::make_unsigned_t<T>>(random()));
            }
            WaveletMatrix<T> matrix{std::span<const T>(values)};
            for (std::size_t i = 0; i < n; ++i) check(matrix[i] == values[i]);
            const std::vector<T> probes{low, static_cast<T>(low + 1), T{0}, T{1},
                                         static_cast<T>(high - 1), high};
            if (n <= 7) {
                for (std::size_t l = 0; l <= n; ++l) for (auto r = l; r <= n; ++r)
                    check_range(matrix, values, l, r, probes);
            } else {
                for (auto [l,r] : std::vector<std::pair<std::size_t,std::size_t>>{
                    {0,n},{0,64},{63,64},{64,64},{63,n},{n,n},{n-1,n}})
                    if (l <= r && r <= n) check_range(matrix, values, l, r, probes);
            }
            auto copy = matrix;
            WaveletMatrix<T> moved(std::move(copy)); check(copy.empty());
            copy.build(std::vector<T>{T{1}}); check(copy[0] == T{1});
            WaveletMatrix<T> assigned; assigned = moved; auto& same = assigned; assigned = same;
            assigned = std::move(same); check(assigned.size() == n);
            for (std::size_t i=0;i<n;++i)check(assigned[i]==values[i]);
            swap(copy, assigned); check(copy.size() == n && assigned.size() == 1);
            assigned.clear(); check(assigned.count(0,0,low) == 0);
            check(!assigned.successor(0,0,low)); check(!assigned.predecessor(0,0,high));
        }
    }
}
int main() {
    std::mt19937_64 random(0x8931bd791);
    exercise<std::uint8_t>(random); exercise<std::int8_t>(random);
    exercise<std::uint16_t>(random); exercise<std::int16_t>(random);
    exercise<std::uint32_t>(random); exercise<std::int32_t>(random);
    exercise<std::uint64_t>(random); exercise<std::int64_t>(random);
    // rankの64bit語境界と、末尾の未使用bitを単独で全位置照合。
    for (std::size_t n : {0,1,63,64,65,127,128,129,255,256,257}) {
        reinsirk::detail::WaveletRankBits bits(n); std::vector<bool> expected(n);
        for (std::size_t i=0;i<n;++i)if((i*37+11)%13<5){bits.set(i);expected[i]=true;}
        bits.finish(); std::size_t count=0;
        for(std::size_t i=0;i<=n;++i){check(bits.rank1(i)==count);if(i<n){check(bits.at(i)==expected[i]);count+=expected[i];}}
    }
    std::cout << "wavelet independent second checks=" << checks << '\n';
}
