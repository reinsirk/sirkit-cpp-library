// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: String/String_Radix_Order.cpp
// 参考（Verifyと辞書順の定義）: https://atcoder.jp/contests/abc219/tasks/abc219_c
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/string_radix_order.md
//
// API一覧
// string_radix_order(words,alphabet={}): 文字列を安定に辞書順で並べる0始まり添字列を返す。
// wordsはstringまたはstring_viewの連続列。空文字列と任意の8bitバイトを許す。
// alphabetは優先順に列挙した異なるバイト。残りは符号なし値の順で続く。重複は検査有効時にinvalid_argument。
// 文字列数n、総バイト数L。256バイト固定のMSD基数ソート。O(n+L+1)時間、O(n+1)追加領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STRING_RADIX_ORDER_DEFINED
#define SIRKIT_STRING_RADIX_ORDER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
using StringRadixDigit = std::uint16_t;
constexpr std::size_t kStringRadixAlphabetSize = 256;

inline auto MakeStringRadixRanks(std::string_view alphabet)
{
    std::array<StringRadixDigit, kStringRadixAlphabetSize> ranks{};
    StringRadixDigit next = 1;
    for (const unsigned char byte : alphabet)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (ranks[byte] != 0)
        {
            throw std::invalid_argument("string_radix_order: repeated alphabet byte");
        }
#endif
        ranks[byte] = next++;
    }
    for (auto &rank : ranks)
    {
        if (rank == 0)
        {
            rank = next++;
        }
    }
    return ranks;
}

template <class Word>
std::vector<std::size_t> SortStringRadixIndices(std::span<const Word> words, std::string_view alphabet)
{
    const auto rank = MakeStringRadixRanks(alphabet);
    std::vector<std::size_t> order(words.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    if (words.size() < 2)
    {
        return order;
    }

    struct Group
    {
        std::size_t begin, end, depth;
    };
    std::vector<Group> active{{0, words.size(), 0}};
    std::vector<std::size_t> dispersed(words.size());
    std::vector<StringRadixDigit> digits(words.size());
    while (!active.empty())
    {
        const auto [begin, end, depth] = active.back();
        active.pop_back();
        std::array<std::size_t, kStringRadixAlphabetSize + 1> counts{};
        StringRadixDigit first = kStringRadixAlphabetSize, last = 0;
        for (auto i = begin; i < end; ++i)
        {
            const auto &word = words[order[i]];
            // 0は終端専用なので、入力の値0も普通のバイトとして扱える。
            const StringRadixDigit digit = depth == word.size() ? 0 : rank[static_cast<unsigned char>(word[depth])];
            digits[i] = digit;
            ++counts[digit];
            first = std::min(first, digit);
            last = std::max(last, digit);
        }
        if (first == last)
        {
            if (first != 0)
            {
                active.push_back({begin, end, depth + 1});
            }
            continue;
        }

        std::array<std::size_t, kStringRadixAlphabetSize + 1> offsets{};
        auto next = begin;
        for (auto digit = first; digit <= last; ++digit)
        {
            offsets[digit] = next;
            next += counts[digit];
        }
        for (auto i = begin; i < end; ++i)
        {
            dispersed[offsets[digits[i]]++] = order[i];
        }
        std::copy(dispersed.begin() + begin, dispersed.begin() + end, order.begin() + begin);

        for (auto digit = std::max(first, StringRadixDigit{1}); digit <= last; ++digit)
        {
            if (counts[digit] > 1)
            {
                const auto high = offsets[digit];
                active.push_back({high - counts[digit], high, depth + 1});
            }
        }
    }
    return order;
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::vector<std::size_t>
string_radix_order(std::span<const std::string_view> words, std::string_view alphabet = {})
{
    return /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SortStringRadixIndices(words, alphabet);
}

[[nodiscard]] inline std::vector<std::size_t>
string_radix_order(std::span<const std::string> words, std::string_view alphabet = {})
{
    return /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::SortStringRadixIndices(words, alphabet);
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
