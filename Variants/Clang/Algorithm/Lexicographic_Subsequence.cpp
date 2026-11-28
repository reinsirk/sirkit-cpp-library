// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Algorithm/Lexicographic_Subsequence.cpp
// 参考（問い合わせの用途）: https://atcoder.jp/contests/typical90/tasks/typical90_f
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lexicographic_subsequence.md
//
// API一覧
// lexicographic_subsequence_indices(values,k,less={}): 長さkの辞書順最小部分列を、0始まりの添字で返す。
// valuesは長さが分かるランダムアクセス範囲。lessは既定でstd::less<>、同値なら添字列も辞書順最小。
// lexicographic_subsequence(text,k): 符号なしバイト順で、長さkの辞書順最小部分列を文字列で返す。
// どちらもO(n+1)時間・O(k+1)領域。0<=k<=n。検査有効時はk>nでinvalid_argument。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LEXICOGRAPHIC_SUBSEQUENCE_DEFINED
#define SIRKIT_LEXICOGRAPHIC_SUBSEQUENCE_DEFINED

template <class Range, class Compare = std::less<>>
    requires std::ranges::random_access_range<const Range> && std::ranges::sized_range<const Range>
        && std::indirect_strict_weak_order<Compare, std::ranges::iterator_t<const Range>>
[[nodiscard]] std::vector<std::size_t> lexicographic_subsequence_indices(const Range &values, std::size_t length,
                                                                      Compare less = {})
{
    const auto n = static_cast<std::size_t>(std::ranges::size(values));
#ifndef SKIP_RUNTIME_CHECKS
    if (length > n)
    {
        throw std::invalid_argument("lexicographic_subsequence_indices: length exceeds input");
    }
#endif
    std::vector<std::size_t> result;
    if (length == 0)
    {
        return result;
    }
    result.reserve(length);
    const auto first = std::ranges::begin(values);
    using Difference = std::ranges::range_difference_t<const Range>;
    for (std::size_t i = 0; i < n; ++i)
    {
        const auto position = static_cast<Difference>(i);
        // 末尾を捨てても必要な個数が残るときだけ、現在値で列を小さくできる。
        while (!result.empty() && n - i > length - result.size()
               && std::invoke(less, first[position], first[static_cast<Difference>(result.back())]))
        {
            result.pop_back();
        }
        if (result.size() < length)
        {
            result.push_back(i);
        }
    }
    return result;
}

[[nodiscard]] inline std::string lexicographic_subsequence(std::string_view text, std::size_t length)
{
    const auto byte_less = [](char left, char right)
    {
        return static_cast<unsigned char>(left) < static_cast<unsigned char>(right);
    };
    const auto indices = lexicographic_subsequence_indices(text, length, byte_less);
    std::string result;
    result.reserve(length);
    for (const auto i : indices)
    {
        result.push_back(text[i]);
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
