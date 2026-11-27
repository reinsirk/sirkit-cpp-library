// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: String/Prefix_Function.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/prefix_function.md
//
// API一覧
// prefix_function(text): 各接頭辞の真の接頭辞かつ接尾辞の最大長。O(n)時間・領域。
// find_occurrences(text,pattern): 一致の開始位置を昇順で返す。重複する出現も含む。
// 空のpatternは境界0からtext.size()までに一致。O(n+m)時間・O(m+k)領域、kは一致数。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <vector>

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
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
