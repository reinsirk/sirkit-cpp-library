// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/String_Periods.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/string_periods.md
//
// API一覧
// StringPeriods(text): 各接頭辞の周期を答えるためにO(n+1)時間・領域で前処理。
// size(): 元の文字列のバイト数。O(1)。
// least_period(length): 先頭lengthバイトの最小周期。末尾は繰り返しの途中で終わってよい。O(1)。
// primitive_root_length(length): 先頭lengthバイトを完全な繰り返しで作る最短のブロック長。O(1)。
// proper_borders(length): 先頭lengthバイトの真の接頭辞かつ接尾辞の長さを降順で返す。長さ0は除く。
// periods(length): 先頭lengthバイトの正の周期を昇順で返す。length自体を含む。
// 列挙はO(k+1)時間・領域で、kは返す要素数。空の接頭辞の最短長は0、列挙結果は空。
// lengthはsize()以下。
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

#ifndef SIRKIT_STRING_PERIODS_DEFINED
#define SIRKIT_STRING_PERIODS_DEFINED

class StringPeriods
{
public:
    explicit StringPeriods(std::string_view text) : border_(prefix_function(text))
    {
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return border_.size();
    }

    [[nodiscard]] std::size_t least_period(std::size_t length) const
    {
        return length - last_border(length);
    }

    [[nodiscard]] std::size_t primitive_root_length(std::size_t length) const
    {
        const auto period = least_period(length);
        if (period == 0 || length % period == 0)
        {
            return period;
        }
        return length;
    }

    [[nodiscard]] std::vector<std::size_t> proper_borders(std::size_t length) const
    {
        std::vector<std::size_t> result;
        for (auto border = last_border(length); border != 0; border = border_[border - 1])
        {
            result.push_back(border);
        }
        return result;
    }

    [[nodiscard]] std::vector<std::size_t> periods(std::size_t length) const
    {
        auto result = proper_borders(length);
        for (auto &border : result)
        {
            border = length - border;
        }
        if (length != 0)
        {
            result.push_back(length);
        }
        return result;
    }

private:
    std::vector<std::size_t> border_;

    [[nodiscard]] std::size_t last_border(std::size_t length) const
    {
        return length == 0 ? 0 : border_[length - 1];
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
