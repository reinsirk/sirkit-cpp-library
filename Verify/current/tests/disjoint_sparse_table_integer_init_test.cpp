// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_DISJOINT_TEST_SOURCE
#define SIRKIT_DISJOINT_TEST_SOURCE "../../../DataStructure/Disjoint_Sparse_Table.cpp"
#endif
#include SIRKIT_DISJOINT_TEST_SOURCE
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

namespace {
std::uint64_t checks = 0;

template<class T, bool Minimum>
struct Select {
    T operator()(T a, T b) const {
        if constexpr (Minimum) return std::min(a, b);
        else return std::max(a, b);
    }
};

template<class T, bool Minimum>
void test(std::size_t n, std::mt19937_64& random) {
    static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);
    static_assert(Minimum || std::is_signed_v<T>);
    constexpr T identity = Minimum ? std::numeric_limits<T>::max() : std::numeric_limits<T>::lowest();
    static_assert(identity != T{});
    std::vector<T> a(n);
    for (auto& value : a) {
        const auto distance = static_cast<T>(random() % 63 + 1);
        if constexpr (Minimum) {
            // 全て正。大きい型の上位bitも使い、0の混入を検出する。
            value = (random() & 1) ? distance : static_cast<T>(std::numeric_limits<T>::max() - distance);
        } else {
            // 全て負。0の混入と符号付き最小値付近の処理を検出する。
            value = (random() & 1) ? static_cast<T>(-distance)
                                   : static_cast<T>(std::numeric_limits<T>::lowest() + distance);
        }
    }
    DisjointSparseTable<T, Select<T, Minimum>> table(a, identity);
    const auto check = [&](bool ok, std::size_t l, std::size_t r) {
        ++checks;
        if (!ok) {
            std::fprintf(stderr, "Disjoint integer init failure: bits=%d signed=%d min=%d n=%zu [%zu,%zu)\n",
                         std::numeric_limits<T>::digits, std::is_signed_v<T>, Minimum, n, l, r);
            std::abort();
        }
    };
    const auto verify = [&](std::size_t l, std::size_t r) {
        T expected = identity;
        for (auto i = l; i < r; ++i) {
            if constexpr (Minimum) {
                if (a[i] < expected) expected = a[i];
            } else {
                if (a[i] > expected) expected = a[i];
            }
        }
        check(table.prod(l, r) == expected, l, r);
        return expected;
    };
    check(table.size() == n && table.empty() == (n == 0), 0, n);
    check(table.values() == a, 0, n);
    check(table.all_prod() == verify(0, n), 0, n);
    for (std::size_t i = 0; i <= n; ++i) {
        verify(i, i);
        if (i < n) {
            check(table.get(i) == a[i], i, i + 1);
            verify(i, i + 1);
        }
    }
    if (n < 20) {
        for (std::size_t l = 0; l <= n; ++l)
            for (auto r = l; r <= n; ++r) verify(l, r);
    }
    // 各段の結合点直前・直後、右端の不完全ブロックを直接含める。
    for (std::size_t half = 1; half < n;) {
        const auto length = std::min(n, half + 1);
        verify(0, length);
        verify(n - length, n);
        for (auto middle = half; middle < n;) {
            verify(middle - 1, middle + 1);
            if (half > (n - middle) / 2) break;
            middle += half * 2;
        }
        if (half > (n - 1) / 2) break;
        half *= 2;
    }
    for (int trial = 0; trial < 250; ++trial) {
        auto l = static_cast<std::size_t>(random() % (n + 1));
        auto r = static_cast<std::size_t>(random() % (n + 1));
        if (l > r) std::swap(l, r);
        verify(l, r);
    }
}

template<class T>
void type_cases(std::mt19937_64& random) {
    for (const auto n : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 16, 17, 19,
                        31, 32, 33, 47, 63, 64, 65, 97, 127, 128, 129, 191,
                        255, 256, 257, 383, 511, 512, 513, 769, 1023, 1024,
                        1025, 1537, 2047, 2048, 2049, 3073, 4095, 4096, 4097,
                        8191, 8192, 8193, 16383, 16384, 16385}) {
        test<T, true>(static_cast<std::size_t>(n), random);
        if constexpr (std::is_signed_v<T>) test<T, false>(static_cast<std::size_t>(n), random);
    }
}
}

int main() {
    std::mt19937_64 random(0x9a15150e6519ULL);
    type_cases<std::int8_t>(random);
    type_cases<std::uint8_t>(random);
    type_cases<std::int16_t>(random);
    type_cases<std::uint16_t>(random);
    type_cases<std::int32_t>(random);
    type_cases<std::uint32_t>(random);
    type_cases<std::int64_t>(random);
    type_cases<std::uint64_t>(random);
    std::printf("Disjoint integer initialization checks=%ju\n", static_cast<std::uintmax_t>(checks));
}
