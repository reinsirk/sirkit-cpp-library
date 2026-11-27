// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: String/Palindromic_Factorization.cpp
// 参考: M. Rubinchik, A. M. Shur, EERTREE: An Efficient Data Structure for Processing Palindromes in Strings, §4.1
// (https://arxiv.org/abs/1506.04862) の series link と最小分割DP。コードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/palindromic_factorization.md
//
// API一覧
// min_palindromic_partition(text): 非空の回文に最少個数で分割し、0とNを含む境界列を返す。
// 空列は{0}。同率ならいずれか。任意のバイトを扱い、添字はバイト位置。
// O(N log(N+1))時間、O(N)領域。N>INT_MAX-1ならlength_error。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PALINDROMIC_FACTORIZATION_DEFINED
#define SIRKIT_PALINDROMIC_FACTORIZATION_DEFINED

namespace detail
{
inline int PalindromicFactorizationSize(std::size_t length)
{
    if (length > static_cast<std::size_t>(std::numeric_limits<int>::max() - 1))
    {
        throw std::length_error("min_palindromic_partition: text is too long");
    }
    return static_cast<int>(length);
}
} // namespace detail

inline std::vector<int> min_palindromic_partition(std::string_view text)
{
    struct Palindrome
    {
        int length = 0;
        int suffix = 0;
        int series = 0;
        int period = 0;
        int first_child = -1;
        int next_sibling = -1;
        int best_prefix = 0;
        char symbol = 0;
    };
    const int n = detail::PalindromicFactorizationSize(text.size());
    constexpr int kOddRoot = 0, kEvenRoot = 1;
    std::vector<Palindrome> nodes;
    nodes.reserve(text.size() + 2);
    nodes.push_back({.length = -1});
    nodes.emplace_back();
    std::vector<int> parts(n + 1), previous(n + 1);

    const auto extendable = [&](int node, int position)
    {
        for (;; node = nodes[node].suffix)
        {
            const int before = position - nodes[node].length - 1;
            if (before >= 0 && text[before] == text[position])
            {
                return node;
            }
        }
    };
    const auto transition = [&](int node, char symbol)
    {
        for (int child = nodes[node].first_child; child >= 0; child = nodes[child].next_sibling)
        {
            if (nodes[child].symbol == symbol)
            {
                return child;
            }
        }
        return -1;
    };

    int longest_suffix = kEvenRoot;
    for (int end = 1; end <= n; ++end)
    {
        const char symbol = text[end - 1];
        const int inside = extendable(longest_suffix, end - 1);
        longest_suffix = transition(inside, symbol);
        if (longest_suffix < 0)
        {
            const int length = nodes[inside].length + 2;
            const int suffix = inside == kOddRoot
                                   ? kEvenRoot
                                   : transition(extendable(nodes[inside].suffix, end - 1), symbol);
            const int period = length - nodes[suffix].length;
            const int series = period == nodes[suffix].period ? nodes[suffix].series : suffix;
            longest_suffix = static_cast<int>(nodes.size());
            nodes.push_back({.length = length,
                             .suffix = suffix,
                             .series = series,
                             .period = period,
                             .next_sibling = nodes[inside].first_child,
                             .symbol = symbol});
            nodes[inside].first_child = longest_suffix;
        }

        int best = end - 1;
        for (int node = longest_suffix; nodes[node].length > 0; node = nodes[node].series)
        {
            auto &palindrome = nodes[node];
            int prefix = end - (nodes[palindrome.series].length + palindrome.period);
            const auto &suffix = nodes[palindrome.suffix];
            // 等差列の最短の回文だけを新たに試す。残りの回文の開始位置は
            // period 個前の接頭辞で処理した suffix の系列と一致する。
            if (palindrome.period == suffix.period && parts[suffix.best_prefix] < parts[prefix])
            {
                prefix = suffix.best_prefix;
            }
            palindrome.best_prefix = prefix;
            if (parts[prefix] < parts[best])
            {
                best = prefix;
            }
        }
        parts[end] = parts[best] + 1;
        previous[end] = best;
    }

    std::vector<int> cuts;
    cuts.reserve(parts[n] + 1);
    for (int end = n; end > 0; end = previous[end])
    {
        cuts.push_back(end);
    }
    cuts.push_back(0);
    std::ranges::reverse(cuts);
    return cuts;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
