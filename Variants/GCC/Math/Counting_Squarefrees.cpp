// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Counting_Squarefrees.cpp
// 数学的参考（Pawlewicz, Algorithm 2）: https://arxiv.org/abs/1107.4890
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/counting_squarefrees.md
//
// API一覧
// count_squarefrees(n): n以下の無平方な正整数の個数。n=0は0。nはuint64_t。
// メビウス関数の線形篩と商をまとめたMertens関数の漸化式を使う。
// 固定幅整数演算をO(1)として、O(n^(2/5))時間・O(n^(2/5))領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COUNTING_SQUAREFREES_DEFINED
#define SIRKIT_COUNTING_SQUAREFREES_DEFINED

namespace detail
{
inline std::uint64_t squarefree_isqrt(std::uint64_t value)
{
    std::uint64_t low = 0, high = std::uint64_t{1} << 32;
    while (high - low > 1)
    {
        const auto middle = (low + high) / 2;
        if (middle * middle <= value)
        {
            low = middle;
        }
        else
        {
            high = middle;
        }
    }
    return low;
}

inline std::uint64_t squarefree_fifth_root(std::uint64_t value)
{
    std::uint64_t low = 0, high = std::uint64_t{1} << 13;
    while (high - low > 1)
    {
        const auto middle = (low + high) / 2;
        const auto square = middle * middle;
        if (middle <= value / (square * square))
        {
            low = middle;
        }
        else
        {
            high = middle;
        }
    }
    return low;
}

inline std::vector<std::int32_t> squarefree_mobius_sieve(std::uint32_t limit)
{
    constexpr std::int32_t kUnvisited = 2;
    std::vector<std::int32_t> mobius(static_cast<std::size_t>(limit) + 1, kUnvisited);
    mobius[0] = 0;
    mobius[1] = 1;
    std::vector<std::uint32_t> primes;
    for (std::uint32_t i = 2; i <= limit; ++i)
    {
        if (mobius[i] == kUnvisited)
        {
            mobius[i] = -1;
            primes.push_back(i);
        }
        // 最小素因数は登録済み。最小素因数を処理するか、積が上限を超えると止まる。
        for (std::size_t j = 0;; ++j)
        {
            const auto prime = primes[j];
            const auto next = std::uint64_t{i} * prime;
            if (next > limit)
            {
                break;
            }
            if (i % prime == 0)
            {
                mobius[next] = 0;
                break;
            }
            mobius[next] = -mobius[i];
        }
    }
    return mobius;
}
} // namespace detail

[[nodiscard]] inline std::uint64_t count_squarefrees(std::uint64_t n)
{
    if (n == 0)
    {
        return 0;
    }

    const auto count_large = detail::squarefree_fifth_root(n);
    const auto limit = static_cast<std::uint32_t>(detail::squarefree_isqrt(n / count_large));
    auto prefix = detail::squarefree_mobius_sieve(limit);
    std::uint64_t result = 0;
    for (std::uint64_t d = 1; d <= limit; ++d)
    {
        if (prefix[d] != 0)
        {
            // 最終結果は[0,n]。途中の加減算は符号なし64ビットの剰余でよい。
            const auto term = n / (d * d);
            result += prefix[d] > 0 ? term : std::uint64_t{0} - term;
        }
        prefix[d] += prefix[d - 1];
    }

    std::vector<std::int64_t> large(count_large);
    std::int64_t correction = -static_cast<std::int64_t>(count_large - 1) * prefix.back();
    for (auto i = count_large - 1; i != 0; --i)
    {
        const auto x = detail::squarefree_isqrt(n / i);
        std::int64_t mertens = 1;
        for (std::uint64_t left = 2; left <= x;)
        {
            const auto quotient = x / left;
            const auto right = x / quotient;
            // floor(sqrt(n/i))/left の下限を取ると floor(sqrt(n/(i*left^2)))。
            const auto value = quotient <= limit ? prefix[quotient] : large[i * left * left];
            mertens -= static_cast<std::int64_t>(right - left + 1) * value;
            left = right + 1;
        }
        large[i] = mertens;
        correction += mertens;
    }
    return result + static_cast<std::uint64_t>(correction);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
