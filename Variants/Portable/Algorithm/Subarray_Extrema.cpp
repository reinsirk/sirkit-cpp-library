// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Algorithm/Subarray_Extrema.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subarray_extrema.md
// 参考（Verify問題）: https://atcoder.jp/contests/agc005/tasks/agc005_b
//
// API一覧
// subarray_extremum_spans(values,before=less): 各位置iが極値を代表する最大範囲[left,right)。同値なら左端を代表にする。
// この範囲内でiを含むすべての部分列がiに対応する。列は長さ付きランダムアクセスRange。
// subarray_extremum_sums_by_length<Sum>(values,before=less): 長さkの全連続部分列の極値の和を添字k-1に返す。
// beforeは狭義弱順序。lessなら最小、greaterなら最大。Sum{}は0、各要素から変換でき、加減算が有効なこと。
// 比較・加減算がO(1)なら各O(n)時間、O(n)領域。空列なら空列。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <functional>
#include <ranges>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBARRAY_EXTREMA_DEFINED
#define SIRKIT_SUBARRAY_EXTREMA_DEFINED
struct SubarrayExtremumSpan
{
    std::size_t left = 0, right = 0;
};

namespace detail
{
template <class Range>
concept SubarrayExtremumRange = std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>;
}

template <detail::SubarrayExtremumRange Range, class Compare = std::less<>>
std::vector<SubarrayExtremumSpan> subarray_extremum_spans(const Range &values, const Compare &before = {})
{
    const auto size = static_cast<std::size_t>(std::ranges::size(values));
    const auto first = std::ranges::begin(values);
    using Distance = std::ranges::range_difference_t<const Range>;
    std::vector<SubarrayExtremumSpan> spans(size);
    std::vector<std::size_t> stack;
    stack.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        while (!stack.empty())
        {
            const auto pivot = stack.back();
            if (!before(first[static_cast<Distance>(i)], first[static_cast<Distance>(pivot)]))
            {
                break;
            }
            spans[pivot].right = i;
            stack.pop_back();
        }
        spans[i] = {stack.empty() ? 0 : stack.back() + 1, size};
        stack.push_back(i);
    }
    return spans;
}

template <class Sum, detail::SubarrayExtremumRange Range, class Compare = std::less<>>
std::vector<Sum> subarray_extremum_sums_by_length(const Range &values, const Compare &before = {})
{
    const auto size = static_cast<std::size_t>(std::ranges::size(values));
    if (size == 0)
    {
        return {};
    }
    std::vector<Sum> result;
    if (size > result.max_size() - 2)
    {
        throw std::length_error("subarray_extremum_sums_by_length: result exceeds table capacity");
    }
    result.resize(size + 2);
    const auto spans = subarray_extremum_spans(values, before);
    const auto first = std::ranges::begin(values);
    using Distance = std::ranges::range_difference_t<const Range>;
    // 左右の端の選び方は長さに対して台形になる。傾きの変化点だけ加算する。
    for (std::size_t i = 0; i < size; ++i)
    {
        const auto value = static_cast<Sum>(first[static_cast<Distance>(i)]);
        const auto left = i - spans[i].left + 1;
        const auto right = spans[i].right - i;
        result[0] += value;
        result[left] -= value;
        result[right] -= value;
        result[left + right] += value;
    }
    Sum slope{}, value{};
    for (std::size_t i = 0; i < size; ++i)
    {
        slope += result[i];
        value += slope;
        result[i] = value;
    }
    result.resize(size);
    return result;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
