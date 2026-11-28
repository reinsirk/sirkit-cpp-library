// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: String/Shortest_Absent_Subsequence.cpp
// 参考（全ての記号を含む区間への貪欲分割）: https://fi.episciences.org/12396
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_absent_subsequence.md
//
// API一覧
// shortest_absent_subsequence(text, alphabet="abcdefghijklmnopqrstuvwxyz"): 部分列として含まれない最短の列を返す。
// 最短が複数なら符号なしバイト辞書順で最小。候補のバイトはalphabetから取り、重複は同じとみなす。
// shortest_absent_subsequence_length(text, alphabet): 最短長だけを返す。同じalphabet既定値。
// alphabetは空でないこと。nはtextの長さ、aはalphabetの長さ。共にO(n+a+1)時間、追加領域は復元O(n+1)、長さのみO(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SHORTEST_ABSENT_SUBSEQUENCE_DEFINED
#define SIRKIT_SHORTEST_ABSENT_SUBSEQUENCE_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline constexpr unsigned kAbsentAlphabetSize = 256;
inline constexpr std::string_view kLowercaseAbsentAlphabet = "abcdefghijklmnopqrstuvwxyz";

struct AbsentAlphabet
{
    std::array<bool, kAbsentAlphabetSize> allowed{};
    std::vector<unsigned char> symbols;

    explicit AbsentAlphabet(std::string_view alphabet)
    {
        if (alphabet.empty())
        {
            throw std::invalid_argument("shortest_absent_subsequence: empty alphabet");
        }
        for (const char byte : alphabet)
        {
            allowed[static_cast<unsigned char>(byte)] = true;
        }
        for (unsigned value = 0; value < kAbsentAlphabetSize; ++value)
        {
            if (allowed[value])
            {
                symbols.push_back(static_cast<unsigned char>(value));
            }
        }
    }
};

inline std::size_t absent_subsequence_slots(std::size_t size)
{
    if (size >= std::vector<std::size_t>().max_size())
    {
        throw std::length_error("shortest_absent_subsequence: text is too long");
    }
    return size + 1;
}

template<class Record>
std::size_t scan_absent_lengths(std::string_view text, const AbsentAlphabet &alphabet, Record record)
{
    std::array<std::size_t, kAbsentAlphabetSize> last_block{};
    std::size_t length = 1;
    auto missing = alphabet.symbols.size();
    for (auto i = text.size(); i != 0;)
    {
        --i;
        const auto current = static_cast<unsigned char>(text[i]);
        if (alphabet.allowed[current] && last_block[current] != length)
        {
            last_block[current] = length;
            if (--missing == 0)
            {
                // 必要な全バイトがそろった区間一つにつき、必ず埋め込める最大長が1増える。
                ++length;
                missing = alphabet.symbols.size();
            }
        }
        record(i, current, length);
    }
    return length;
}
} // namespace reinsirk::detail

inline std::size_t shortest_absent_subsequence_length(std::string_view text,
                                                       std::string_view alphabet = detail::kLowercaseAbsentAlphabet)
{
    const detail::AbsentAlphabet letters(alphabet);
    return detail::scan_absent_lengths(text, letters, [](std::size_t, unsigned char, std::size_t) {});
}

inline std::string shortest_absent_subsequence(std::string_view text,
                                                std::string_view alphabet = detail::kLowercaseAbsentAlphabet)
{
    const detail::AbsentAlphabet letters(alphabet);
    std::vector<std::size_t> lengths(detail::absent_subsequence_slots(text.size()), 1);
    std::array<std::vector<std::size_t>, detail::kAbsentAlphabetSize> positions;
    const auto length = detail::scan_absent_lengths(text, letters, [&](std::size_t i, unsigned char current, std::size_t value) {
        lengths[i] = value;
        if (letters.allowed[current])
        {
            positions[current].push_back(i);
        }
    });
    std::string result;
    result.reserve(length);
    std::size_t start = 0;
    while (true)
    {
        // 最短の答えの先頭にできるバイトは必ずあり、その中の最小値を選ぶ。
        for (std::size_t k = 0;; ++k)
        {
            const auto symbol = letters.symbols[k];
            auto &row = positions[symbol];
            while (!row.empty() && row.back() < start)
            {
                row.pop_back();
            }
            if (row.empty())
            {
                result.push_back(static_cast<char>(symbol));
                return result;
            }
            const auto following = row.back() + 1;
            if (lengths[following] + 1 == lengths[start])
            {
                result.push_back(static_cast<char>(symbol));
                start = following;
                break;
            }
        }
    }
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
