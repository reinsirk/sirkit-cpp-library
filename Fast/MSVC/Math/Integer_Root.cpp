// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Integer_Root.cpp
// 参考（用途）: https://ei1333.github.io/library/math/number-theory/kth-root-integer.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/integer_root.md
//
// API一覧
// integer_kth_root(n,k): uint64_tのnに対し、x^k<=nを満たす最大の非負整数x。k>=1。
// O(1+ceil(log(n+2)/k)*log(k+1))時間・O(1)領域。近似から探し、整数演算で確定する。
// 検査有効時、k=0ならstd::invalid_argument。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_INTEGER_ROOT_DEFINED
#define SIRKIT_INTEGER_ROOT_DEFINED

namespace detail
{
inline bool IntegerPowerAtMost(std::uint64_t base, unsigned exponent, std::uint64_t limit)
{
    std::uint64_t product = 1;
    while (true)
    {
        if ((exponent & 1) != 0)
        {
            if (product > limit / base)
            {
                return false;
            }
            product *= base;
        }
        exponent >>= 1;
        if (exponent == 0)
        {
            return true;
        }
        if (base > limit / base)
        {
            return false;
        }
        base *= base;
    }
}
inline std::uint64_t RefineIntegerRoot(std::uint64_t n, unsigned k, std::uint64_t guess, std::uint64_t upper)
{
    std::uint64_t low = 1, high = upper;
    if (IntegerPowerAtMost(guess, k, n))
    {
        if (!IntegerPowerAtMost(guess + 1, k, n))
        {
            return guess;
        }
        low = guess + 1;
    }
    else
    {
        if (IntegerPowerAtMost(guess - 1, k, n))
        {
            return guess - 1;
        }
        high = guess - 1;
    }
    while (high - low > 1)
    {
        const auto middle = low + (high - low) / 2;
        if (IntegerPowerAtMost(middle, k, n))
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
} // namespace detail

inline std::uint64_t integer_kth_root(std::uint64_t n, unsigned k)
{
    if (n < 2 || k == 1)
    {
        return n;
    }
    const unsigned bits = std::bit_width(n);
    if (k >= bits)
    {
        return 1;
    }
    const unsigned upper_bits = (bits + k - 1) / k;
    const auto upper = std::uint64_t{1} << upper_bits;
    const double value = static_cast<double>(n);
    const double estimate = k == 2   ? std::sqrt(value)
                            : k == 3 ? std::cbrt(value)
                                     : std::pow(value, 1.0 / static_cast<double>(k));
    const auto guess = static_cast<std::uint64_t>(std::clamp(estimate, 1.0, static_cast<double>(upper - 1)));
    return detail::RefineIntegerRoot(n, k, guess, upper);
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
