// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef WAVELET_SOURCE
#define WAVELET_SOURCE "../../../DataStructure/Wavelet_Matrix.cpp"
#endif
#include WAVELET_SOURCE
#include WAVELET_SOURCE
#include <cassert>
#include <iostream>
#include <random>

namespace {
std::uint64_t checks = 0;
void check(bool value) { ++checks; assert(value); }
template <class T>
T random_value(std::mt19937_64& random) {
    return std::bit_cast<T>(static_cast<std::make_unsigned_t<T>>(random()));
}
template <class T>
void test_type(std::mt19937_64& random) {
    for (std::size_t n : {0,1,2,3,7,31,32,33,63,64,65,127,128,129,255,256,257,1024}) {
        for (int mode = 0; mode < 4; ++mode) {
            std::vector<T> values(n);
            for (std::size_t i = 0; i < n; ++i)
                values[i] = mode == 0 ? T{0} : mode == 1 ? std::numeric_limits<T>::lowest() :
                    mode == 2 ? static_cast<T>(i % 7) : random_value<T>(random);
            if (n >= 2 && mode == 3) {
                values[0] = std::numeric_limits<T>::lowest();
                values[1] = std::numeric_limits<T>::max();
            }
            WaveletMatrix<T> matrix(values);
            check(matrix.size() == n); check(matrix.empty() == values.empty());
            for (std::size_t i = 0; i < n; ++i) check(matrix.access(i) == values[i]);
            for (int query = 0; query < 200; ++query) {
                auto left = static_cast<std::size_t>(random() % (n + 1));
                auto right = static_cast<std::size_t>(random() % (n + 1));
                if (left > right) std::swap(left, right);
                std::vector<T> sorted(values.begin() + left, values.begin() + right);
                std::sort(sorted.begin(), sorted.end());
                const T x = query % 5 == 0 ? std::numeric_limits<T>::lowest() :
                    query % 5 == 1 ? std::numeric_limits<T>::max() : random_value<T>(random);
                const T upper = random_value<T>(random);
                const auto low = std::lower_bound(sorted.begin(), sorted.end(), x);
                const auto high = std::upper_bound(sorted.begin(), sorted.end(), x);
                check(matrix.count_less(left,right,x) == static_cast<std::size_t>(low - sorted.begin()));
                check(matrix.count_less_equal(left,right,x) == static_cast<std::size_t>(high - sorted.begin()));
                check(matrix.rank(x,left,right) == static_cast<std::size_t>(high-low));
                check(matrix.count(left,right,x) == static_cast<std::size_t>(high-low));
                check(matrix.range_freq(left,right,x,upper) == (x < upper ? static_cast<std::size_t>(
                    std::lower_bound(sorted.begin(), sorted.end(), upper) - low) : 0));
                check(matrix.predecessor(left,right,x) == (low == sorted.begin() ? std::nullopt : std::optional<T>(*(low-1))));
                check(matrix.successor(left,right,x) == (low == sorted.end() ? std::nullopt : std::optional<T>(*low)));
                if (!sorted.empty()) {
                    const auto k = static_cast<std::size_t>(random() % sorted.size());
                    check(matrix.kth_smallest(left,right,k) == sorted[k]);
                    check(matrix.kth_largest(left,right,k) == sorted[sorted.size()-1-k]);
                    check(matrix.range_min(left,right) == sorted.front());
                    check(matrix.range_max(left,right) == sorted.back());
                }
            }
            auto copy = matrix;
            copy.build(std::vector<T>{T{1}});
            check(copy.size() == 1 && copy[0] == T{1}); check(matrix.size() == n);
            WaveletMatrix<T> assigned; assigned = matrix;
            for (std::size_t i = 0; i < n; ++i) check(assigned[i] == values[i]);
            assigned = assigned;
            auto moved = std::move(assigned); check(assigned.empty()); check(moved.size() == n);
            copy = std::move(moved); check(moved.empty()); check(copy.size() == n);
            if (n != 0) check(copy[0] == values[0]);
            copy.clear(); check(copy.empty());
#ifndef SKIP_RUNTIME_CHECKS
            bool thrown = false;
            try { matrix.access(n); } catch (const std::out_of_range&) { thrown = true; }
            check(thrown); thrown = false;
            try { matrix.count_less(0,n+1,T{0}); } catch (const std::out_of_range&) { thrown = true; }
            check(thrown); thrown = false;
            try { matrix.kth_smallest(0,n,n); } catch (const std::out_of_range&) { thrown = true; }
            check(thrown);
#endif
        }
    }
}
}
int main() {
    std::mt19937_64 random(0xc13fa9a902a6328f);
    test_type<std::int8_t>(random); test_type<std::uint8_t>(random);
    test_type<std::int16_t>(random); test_type<std::uint16_t>(random);
    test_type<std::int32_t>(random); test_type<std::uint32_t>(random);
    test_type<std::int64_t>(random); test_type<std::uint64_t>(random);
    std::cout << "wavelet matrix checks=" << checks << '\n';
}
