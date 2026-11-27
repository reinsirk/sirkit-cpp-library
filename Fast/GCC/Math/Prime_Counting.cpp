// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Prime_Counting.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/prime_counting.md
//
// API一覧
// count_primes(n): n以下の素数の個数。n<2なら0。
// 異なる商 floor(n/i) をふるい、O(n^(3/4))時間・O(√n)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COUNT_PRIMES_DEFINED
#define SIRKIT_COUNT_PRIMES_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t prime_count_table_size(std::uint64_t root, std::size_t capacity)
{
    if (root >= capacity)
    {
        throw std::length_error("count_primes: table too large");
    }
    return static_cast<std::size_t>(root) + 1;
}
} // namespace detail

inline std::uint64_t count_primes(std::uint64_t n)
{
    if (n < 2)
    {
        return 0;
    }
    // 平方根の上界から整数Newton法を始める。
    auto root = std::uint64_t{1} << ((std::bit_width(n) + 1) / 2);
    for (;;)
    {
        const auto next = (root + n / root) / 2;
        if (next >= root)
        {
            break;
        }
        root = next;
    }
    const auto capacity = detail::prime_count_table_size(root, std::vector<std::uint64_t>{}.max_size());
    std::vector<std::uint64_t> small(capacity), large(capacity);
    for (std::uint64_t i = 1; i <= root; ++i)
    {
        small[i] = i - 1;
        large[i] = n / i - 1;
    }
    for (std::uint64_t p = 2; p <= root; ++p)
    {
        const auto below = small[p - 1];
        if (small[p] == below)
        {
            continue;
        }
        // floor(floor(n/i)/p) = floor(n/(i*p))。被参照項を更新前に読む。
        const auto square = p * p;
        const auto last = std::min(root, n / square);
        for (std::uint64_t i = 1; i <= last; ++i)
        {
            const auto index = i * p;
            const auto source = index <= root ? large[index] : small[n / index];
            large[i] -= source - below;
        }
        for (auto i = root; i >= square; --i)
        {
            small[i] -= small[i / p] - below;
        }
    }
    return large[1];
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
