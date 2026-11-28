// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Pattern_Range_Occurrences.cpp
// 参考（区間内の出現）: https://atcoder.jp/contests/abc477/tasks/abc477_c
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pattern_range_occurrences.md
//
// API一覧
// PatternRangeOccurrences(text,pattern): 固定パターンの出現を前計算。長さn,mに対しO(1+n+m)時間・領域。
// size()/pattern_size() const: 文字列とパターンの長さ。O(1)。
// count(l,r) const: 半開区間[l,r)全体に収まる出現数。O(1)。
// positions(l,r) const: 同じ出現の開始位置を昇順の借用spanで返す。O(1)。重なりも数える。
// 0始まり・バイト比較。空パターンは各境界l..rに出現。コピーは内部の読み取り専用の表を共有する。
// 既定構築・move元は両方が空の状態。借用spanは表を所有する実体が残る間有効。
//
// [同梱の接頭辞関数]
// prefix_function(text): 各接頭辞の真の接頭辞かつ接尾辞の最大長。O(n)時間・領域。
// find_occurrences(text,pattern): 一致の開始位置を昇順で返す。重複する出現も含む。
// 空のpatternは境界0からtext.size()までに一致。O(n+m)時間・O(m+k)領域、kは一致数。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <array>
#include <memory>
#include <span>


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

#ifndef SIRKIT_PATTERN_RANGE_OCCURRENCES_DEFINED
#define SIRKIT_PATTERN_RANGE_OCCURRENCES_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t PatternRangeOffsetCount(std::size_t length, std::size_t capacity)
{
    if (capacity < 2 || length > capacity - 2)
    {
        throw std::length_error("PatternRangeOccurrences: text too long");
    }
    return length + 2;
}
} // namespace reinsirk::detail

class PatternRangeOccurrences
{
    struct Data
    {
        std::size_t length, pattern_length;
        std::vector<std::size_t> offsets, starts;

        Data(std::string_view text, std::string_view pattern)
            : length(text.size()), pattern_length(pattern.size())
        {
            offsets.resize(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::PatternRangeOffsetCount(length, offsets.max_size()));
            starts = find_occurrences(text, pattern);
            for (const auto position : starts)
            {
                ++offsets[position + 1];
            }
            std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());
        }
    };

    static constexpr std::array<std::size_t, 1> kEmptyBoundary{0};
    std::shared_ptr<const Data> data_;

public:
    PatternRangeOccurrences() = default;

    PatternRangeOccurrences(std::string_view text, std::string_view pattern)
        : data_(std::make_shared<const Data>(text, pattern))
    {
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return data_ ? data_->length : 0;
    }

    [[nodiscard]] std::size_t pattern_size() const noexcept
    {
        return data_ ? data_->pattern_length : 0;
    }

    [[nodiscard]] std::span<const std::size_t> positions(std::size_t left, std::size_t right) const
    {
        if (!data_)
        {
            return kEmptyBoundary;
        }
        const auto &data = *data_;
        if (data.pattern_length > right - left)
        {
            return {};
        }

        // offsets[k]はkより前に始まる出現数。最右の開始位置はright-pattern_length。
        const auto first = data.offsets[left];
        const auto stop = data.offsets[right - data.pattern_length + 1];
        return std::span<const std::size_t>(data.starts).subspan(first, stop - first);
    }

    [[nodiscard]] std::size_t count(std::size_t left, std::size_t right) const
    {
        return positions(left, right).size();
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
