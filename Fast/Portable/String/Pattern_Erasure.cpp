// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Pattern_Erasure.cpp
// 参考（左端の一致を繰り返し削除する問題）: https://atcoder.jp/contests/abc328/tasks/abc328_d
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pattern_erasure.md
//
// API一覧
// erase_pattern(text,pattern): 左端の連続一致を繰り返し消した文字列を返す。
// erase_pattern_inplace(text,pattern): textを書き換え、削除された出現回数を返す。patternがtext内を参照してもよい。
// バイト単位で比較し、空patternはinvalid_argument。どちらもO(n+m+1)時間・O(n+m+1)領域。
//
// [同梱のPrefix Function]
// prefix_function(text): 各接頭辞の真の接頭辞かつ接尾辞の最大長。O(n)時間・領域。
// find_occurrences(text,pattern): 一致の開始位置を昇順で返す。重複する出現も含む。
// 空のpatternは境界0からtext.size()までに一致。O(n+m)時間・O(m+k)領域、kは一致数。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <string>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PREFIX_FUNCTION_DEFINED
#define SIRKIT_PREFIX_FUNCTION_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t occurrence_boundary_count(std::size_t size, std::size_t capacity)
{
    if (size >= capacity)
    {
        throw std::length_error("find_occurrences: too many occurrences");
    }
    return size + 1;
}
} // namespace detail

inline std::vector<std::size_t> prefix_function(std::string_view text)
{
    std::vector<std::size_t> length(text.size());
    for (std::size_t i = 1; i < text.size(); ++i)
    {
        auto previous = length[i - 1];
        while (previous != 0 && text[i] != text[previous])
        {
            previous = length[previous - 1];
        }
        length[i] = previous + (text[i] == text[previous]);
    }
    return length;
}

inline std::vector<std::size_t> find_occurrences(std::string_view text, std::string_view pattern)
{
    std::vector<std::size_t> result;
    if (pattern.empty())
    {
        result.resize(detail::occurrence_boundary_count(text.size(), result.max_size()));
        std::iota(result.begin(), result.end(), std::size_t{0});
        return result;
    }

    const auto border = prefix_function(pattern);
    std::size_t matched = 0;
    for (std::size_t i = 0; i < text.size(); ++i)
    {
        while (matched != 0 && text[i] != pattern[matched])
        {
            matched = border[matched - 1];
        }
        matched += text[i] == pattern[matched];
        if (matched == pattern.size())
        {
            result.push_back(i - (matched - 1));
            matched = border[matched - 1];
        }
    }
    return result;
}

#endif

#ifndef SIRKIT_PATTERN_ERASURE_DEFINED
#define SIRKIT_PATTERN_ERASURE_DEFINED

inline std::size_t erase_pattern_inplace(std::string &text, std::string_view pattern)
{
    const auto length = pattern.size();
    if (length == 0)
    {
        throw std::invalid_argument("erase_pattern: empty pattern");
    }
    const auto input_size = text.size();
    if (length > input_size)
    {
        return 0;
    }

    // 書き換えるtext内を指すpatternも、先にコピーして参照先を保つ。
    const std::string needle(pattern);
    const auto border = prefix_function(needle);
    std::vector<std::size_t> states(input_size);
    std::size_t write = 0, matched = 0;
    for (std::size_t read = 0; read < input_size; ++read)
    {
        const char byte = text[read];
        while (matched != 0 && byte != needle[matched])
        {
            matched = border[matched - 1];
        }
        matched += byte == needle[matched];
        if (matched == length)
        {
            write -= length - 1;
            matched = write == 0 ? 0 : states[write - 1];
        }
        else
        {
            text[write] = byte;
            states[write] = matched;
            ++write;
        }
    }
    text.resize(write);
    return (input_size - write) / length;
}

[[nodiscard]] inline std::string erase_pattern(std::string_view text, std::string_view pattern)
{
    std::string result(text);
    erase_pattern_inplace(result, pattern);
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
