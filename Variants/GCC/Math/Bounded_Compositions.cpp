// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Bounded_Compositions.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bounded_compositions.md
// 参考（Verify問題）: https://atcoder.jp/contests/dp/tasks/dp_m
//
// API一覧
// bounded_composition_counts(bounds,K,mod=998244353): 各整数範囲から1個選び、合計sの方法数を添字sに返す。0<=s<=K。
// boundsはBoundedCompositionPart{lower,upper}の列、または下限0のstd::size_t上限列。両端を含む。
// modは正のuint32_t。素数でなくてよい。範囲なしの合計0は1通り。法modでの剰余を返す。
// 範囲がn個ならO((n+1)(K+1))時間、O(K+1)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BOUNDED_COMPOSITIONS_DEFINED
#define SIRKIT_BOUNDED_COMPOSITIONS_DEFINED
struct BoundedCompositionPart
{
    std::size_t lower = 0, upper = 0;
};

namespace detail
{
inline std::uint32_t AddCompositionResidues(std::uint32_t a, std::uint32_t b, std::uint32_t modulus) noexcept
{
    const auto sum = std::uint64_t{a} + b;
    return static_cast<std::uint32_t>(sum >= modulus ? sum - modulus : sum);
}

inline std::uint32_t SubtractCompositionResidues(std::uint32_t a, std::uint32_t b, std::uint32_t modulus) noexcept
{
    return a >= b ? a - b : static_cast<std::uint32_t>(std::uint64_t{a} + modulus - b);
}

template <class Bounds>
std::vector<std::uint32_t> BoundedCompositionCounts(const Bounds &bounds, std::size_t max_sum, std::uint32_t modulus)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (modulus == 0)
    {
        throw std::invalid_argument("bounded_composition_counts: positive modulus required");
    }
#endif
    std::vector<std::uint32_t> result;
    if (max_sum >= result.max_size())
    {
        throw std::length_error("bounded_composition_counts: result exceeds table capacity");
    }
    const auto length = max_sum + 1;
    std::size_t minimum = 0;
    for (const auto &[lower, upper] : bounds)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (lower > upper)
        {
            throw std::invalid_argument("bounded_composition_counts: invalid part bounds");
        }
#endif
        minimum += std::min(lower, length - minimum);
    }
    result.resize(length);
    if (minimum > max_sum || modulus == 1)
    {
        return result;
    }
    auto coefs = std::span<std::uint32_t>(result).subspan(minimum);
    coefs[0] = 1;
    const auto max_shift = max_sum - minimum;
    std::size_t active = 0;
    for (const auto &[lower, upper] : bounds)
    {
        const auto width = upper - lower;
        if (width == 0)
        {
            continue;
        }
        active += std::min(width, max_shift - active);
        // (1+X+...+X^width) を累積和と、width+1個ずらした差で掛ける。
        for (std::size_t i = 1; i <= active; ++i)
        {
            coefs[i] = AddCompositionResidues(coefs[i], coefs[i - 1], modulus);
        }
        for (auto i = active; i > width; --i)
        {
            coefs[i] = SubtractCompositionResidues(coefs[i], coefs[i - width - 1], modulus);
        }
    }
    return result;
}
}

inline std::vector<std::uint32_t> bounded_composition_counts(std::span<const BoundedCompositionPart> bounds,
                                                             std::size_t max_sum, std::uint32_t modulus = 998244353)
{
    return detail::BoundedCompositionCounts(bounds, max_sum, modulus);
}

inline std::vector<std::uint32_t> bounded_composition_counts(std::span<const std::size_t> upper,
                                                             std::size_t max_sum, std::uint32_t modulus = 998244353)
{
    const auto bounds = upper | std::views::transform([](std::size_t limit)
                                                      {
                                                          return BoundedCompositionPart{0, limit};
                                                      });
    return detail::BoundedCompositionCounts(bounds, max_sum, modulus);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
