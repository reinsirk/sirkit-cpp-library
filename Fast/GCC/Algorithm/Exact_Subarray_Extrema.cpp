// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Exact_Subarray_Extrema.cpp
// 参考（最小値と最大値の指定）: https://atcoder.jp/contests/abc247/tasks/abc247_e
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/exact_subarray_extrema.md
//
// API一覧
// ExactSubarrayExtremaResult{count,left,right}: 有効な区間数と最長の半開区間。0始まり。同長なら左端最小。
// analyze_subarrays_with_extrema(values,minimum,maximum,compare=std::less<>{}):
// 最小値と最大値が指定値に等しい空でない連続部分列を数え、最長区間も返す。なければ{0,0,0}。
// compareは狭義弱順序。minimumが先、maximumが後または等価。O(n+1)時間・O(1)領域。個数はuint64_tで表せること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <ranges>
#include <stdexcept>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EXACT_SUBARRAY_EXTREMA_DEFINED
#define SIRKIT_EXACT_SUBARRAY_EXTREMA_DEFINED

struct ExactSubarrayExtremaResult
{
    std::uint64_t count = 0;
    std::size_t left = 0;
    std::size_t right = 0;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::uint64_t AddExactExtremaCount(std::uint64_t count, std::size_t amount)
{
    return count + amount;
}
} // namespace reinsirk::detail

template <typename Range, typename T, typename Compare = std::less<>>
    requires std::ranges::input_range<const Range>
[[nodiscard]] ExactSubarrayExtremaResult analyze_subarrays_with_extrema(
    const Range &values, const T &minimum, const T &maximum, const Compare &compare = {})
{
    ExactSubarrayExtremaResult result;
    std::size_t last_minimum = 0, last_maximum = 0, left = 0, right = 0;
    for (const auto &value : values)
    {
        ++right;
        if (std::invoke(compare, value, minimum) || std::invoke(compare, maximum, value))
        {
            left = right;
            continue;
        }
        if (!std::invoke(compare, minimum, value))
        {
            last_minimum = right;
        }
        if (!std::invoke(compare, value, maximum))
        {
            last_maximum = right;
        }

        // 最新の範囲外要素より後に始まり、両方の極値を含む左端だけが有効。
        const auto stop = std::min(last_minimum, last_maximum);
        if (stop > left)
        {
            result.count = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::AddExactExtremaCount(result.count, stop - left);
            if (right - left > result.right - result.left)
            {
                result.left = left;
                result.right = right;
            }
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
