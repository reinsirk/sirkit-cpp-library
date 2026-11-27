// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: ローカル独立検査

// Library: String/Palindromic_Factorization.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
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


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_PALINDROMIC_FACTORIZATION_DEFINED
#define SIRKIT_PALINDROMIC_FACTORIZATION_REFERENCE_SOURCE "../../../String/Palindromic_Factorization.cpp"
#include SIRKIT_PALINDROMIC_FACTORIZATION_REFERENCE_SOURCE
#undef SIRKIT_PALINDROMIC_FACTORIZATION_REFERENCE_SOURCE
#endif
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
bool is_palindrome(std::string_view text)
{
    return std::equal(text.begin(), text.begin() + text.size() / 2, text.rbegin());
}

void compare(std::string_view text)
{
    const auto answer = sirkit::min_palindromic_partition(text);
    assert(!answer.empty() && answer.front() == 0 && answer.back() == static_cast<int>(text.size()));
    for (std::size_t i = 1; i < answer.size(); ++i)
    {
        assert(answer[i - 1] < answer[i]);
        assert(is_palindrome(text.substr(answer[i - 1], answer[i] - answer[i - 1])));
    }

    // 列挙した全回文を有向辺とみなして、接頭辞の長さの順に最短距離を求める。
    std::vector<std::size_t> distance(text.size() + 1, text.size());
    distance[0] = 0;
    for (std::size_t start = 0; start < text.size(); ++start)
    {
        for (std::size_t end = start + 1; end <= text.size(); ++end)
        {
            if (is_palindrome(text.substr(start, end - start)))
            {
                distance[end] = std::min(distance[end], distance[start] + 1);
            }
        }
    }
    assert(answer.size() - 1 == distance.back());
}

void enumerate(std::string &text, int remaining, std::string_view alphabet)
{
    compare(text);
    if (remaining == 0)
    {
        return;
    }
    for (const char symbol : alphabet)
    {
        text.push_back(symbol);
        enumerate(text, remaining - 1, alphabet);
        text.pop_back();
    }
}
} // namespace

int main()
{
    std::string text;
    enumerate(text, 14, "ab");
    enumerate(text, 10, std::string_view{"\0\x80\xff", 3});
    std::mt19937 random(1439);
    for (int i = 0; i < 1500; ++i)
    {
        const unsigned alphabet = 1u + random() % 256;
        text.resize(random() % 120);
        for (auto &symbol : text)
        {
            symbol = static_cast<char>(random() % alphabet);
        }
        compare(text);
    }

    constexpr int kN = (1 << 19) - 1;
    text.resize(kN);
    for (unsigned i = 0; i < text.size(); ++i)
    {
        // Zimin word は接尾回文の系列数が大きい。
        text[i] = static_cast<char>(std::countr_zero(i + 1));
    }
    assert(sirkit::min_palindromic_partition(text) == (std::vector<int>{0, kN}));
    text.push_back('\xff');
    assert(sirkit::min_palindromic_partition(text) == (std::vector<int>{0, kN, kN + 1}));
    text.assign(kN, '\0');
    assert(sirkit::min_palindromic_partition(text) == (std::vector<int>{0, kN}));

    for (unsigned i = 0; i < text.size(); ++i)
    {
        // 長さ2以上の回文がない全バイトの周期入力。
        text[i] = static_cast<char>(i % 256);
    }
    const auto separate = sirkit::min_palindromic_partition(text);
    assert(separate.size() == text.size() + 1);
    for (int i = 0; i <= kN; ++i)
    {
        assert(separate[i] == i);
    }

    bool failed = false;
    try
    {
        (void)sirkit::detail::PalindromicFactorizationSize(std::numeric_limits<std::size_t>::max());
    }
    catch (const std::length_error &)
    {
        failed = true;
    }
    assert(failed);
}
