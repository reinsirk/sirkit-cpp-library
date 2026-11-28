// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Lexicographic_Concatenation.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lexicographic_concatenation.md
// 参考（並べ替えの原理）: https://info.atcoder.jp/entry/algorithm_lectures/lex_min_string_concatenation
//
// API一覧
// lexicographic_concatenation_order(words): 連結結果が辞書順最小となる添字の並びを返す。
// second_lexicographic_concatenation(words): 全順列での連結結果を重複ごと数え、辞書順で2番目を返す。2本未満はnullopt。
// 符号なしバイト順。空文字列も可。n個、総長LでO(n log(n+1)+L)時間、O(n+L+1)領域。L<=INT_MAX-2。
//
// [同梱の部分文字列比較]
// SubstringComparison(text): O(n+1)時間・領域で前処理。入力の保存は不要。長さはINT_MAX-2以下。
// size(): 元の文字列のバイト数。
// common_prefix(first,second): 2つの接尾辞の最長共通接頭辞の長さ。末尾位置は空の接尾辞。
// equal(first,second,length): 指定位置から同じ長さの部分文字列が一致するか。
// compare(first,first_length,second,second_length): 部分文字列を辞書順で比較。バイトは符号なしの順序。
// 添字は0始まり。開始位置と長さは元の文字列に収まること。各照会はO(1)、コピー構築は前処理結果を共有。
//
// [同梱の接尾辞配列]
// suffix_array(const std::string&)/suffix_array(string_view): 符号なしバイトの辞書順で接尾辞の開始位置を返す。空接尾辞は含めない。
// C文字列のsuffix_array(char*/const char*)も可。終端あり・nullptr不可。文字列を複製しない。
// suffix_array(vector<int>, upper): 全要素0..upperの整数列。SA-ISでO(n+upper)時間・領域。
// suffix_array<T>(const vector<T>&): Tの<で圧縮して構築。負数/uint64_t最大値も可、O(n log n)時間・O(n)領域。
// upperは0以上INT_MAX-1未満。文字列版はO(n)時間・領域。全版で列長はINT_MAX-1未満、空なら{}。返却添字はint。
// lcp_array(const string&,sa)/lcp_array<T>(const vector<T>&,sa): 隣り合う接尾辞の共通接頭辞長を返す。n<2なら{}、それ以外はn-1個。O(n)時間・領域。
// 追加のlcp_array(sequence,sa)はstring_view/array等にも対応。
// lcp_arrayのsaはsの正しい接尾辞配列が前提。既定検査はサイズ・添字・重複を確認し、辞書順までは再検査しない。
// SuffixArrayEqualRange(s,sa,pattern): patternを接頭辞に持つ接尾辞のsa内半開区間を返す。
// 検索はO((m+1)log(n+1))時間・追加O(1)。string_view版は符号なしバイト順、整数vector版はTの順序。
// 各比較/添字アクセスがO(1)の場合の計算量。Tの<は一貫した順序が前提。
// 空patternは全接尾辞に一致。検索はsaの妥当性を全走査せず、実際に参照する添字だけ検査する。
//
// [同梱の静的区間最小値]
// StaticRangeMinimum<T,Compare>(values,compare={}): 更新のない配列を前処理。O(n+1)時間・領域。
// Compare(a,b)がtrueならaを選ぶ。既定はstd::less<>。比較は狭義弱順序で、比較演算をO(1)とする。
// size(): 配列長。argmin(l,r): 半開区間[l,r)の最小値の最初の添字。min(l,r): 最小値への参照（T=boolは値）。
// 0<=l<r<=size()。size、argmin、minはO(1)。コピーは前処理結果を共有する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <bit>
#include <functional>
#include <compare>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUFFIX_ARRAY_DETAIL_DEFINED
#define SIRKIT_SUFFIX_ARRAY_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline void suffix_length_check(std::size_t n) {
    if (n >= static_cast<std::size_t>(std::numeric_limits<int>::max() - 1))
        throw std::length_error("suffix_array: sequence too long");
}

// 正の整数列に一意な終端0を付けた列を扱う。再帰先にも終端0を残す。
template<class Symbol> std::vector<int> suffix_induced(std::span<const Symbol> symbols, int alphabet) {
    const int n = static_cast<int>(symbols.size());
    if (n == 1) return {0};
    // 0=L型、1=S型、3=LMS。隣接2位置の型を決めてから境界印を重ねる。
    std::vector<std::uint8_t> small(n);
    std::vector<int> selected;
    selected.reserve(static_cast<std::size_t>(n / 2));
    std::vector<int> starts(static_cast<std::size_t>(alphabet) + 1), ends(starts.size());
    ++ends[symbols[n - 1]];
    small[n - 1] = 1;
    for (int i = n - 2; i >= 0; --i) {
        ++ends[symbols[i]];
        small[i] = symbols[i] < symbols[i + 1] ||
            (symbols[i] == symbols[i + 1] && small[i + 1]);
        if (!small[i] && small[i + 1]) { small[i + 1] |= 2; selected.push_back(i + 1); }
    }
    std::reverse(selected.begin(), selected.end());
    int total = 0;
    for (int symbol = 0; symbol <= alphabet; ++symbol) {
        starts[symbol] = total; total += ends[symbol]; ends[symbol] = total;
    }
    std::vector<int> suffixes(n, -1), cursor(ends.size());
    const auto induce = [&](const std::vector<int>& order, bool clear) {
        if (clear) std::fill(suffixes.begin(), suffixes.end(), -1);
        cursor = ends;
        for (auto it = order.rbegin(); it != order.rend(); ++it)
            suffixes[--cursor[symbols[*it]]] = *it;
        cursor = starts;
        for (int slot = 0; slot < n; ++slot) {
            const int before = suffixes[slot] - 1;
            if (before >= 0 && !small[before])
                suffixes[cursor[symbols[before]]++] = before;
        }
        cursor = ends;
        for (int slot = n - 1; slot >= 0; --slot) {
            const int before = suffixes[slot] - 1;
            if (before >= 0 && small[before])
                suffixes[--cursor[symbols[before]]] = before;
        }
    };
    induce(selected, false);
    const int count = static_cast<int>(selected.size());
    // LMSが終端だけなら、再誘導しても同じ順序になる。
    if (count == 1) return suffixes;
    std::vector<int> reduced(count);
    // LMSは隣接しないため位置/2が一意。LMSの欄だけ設定・参照する。
    auto selected_id = std::make_unique_for_overwrite<int[]>(static_cast<std::size_t>((n + 1) / 2));
    for (int i = 0; i < count; ++i) selected_id[selected[i] / 2] = i;
    int previous = -1, name = -1;
    for (int position : suffixes) {
        if ((small[position] & 2) == 0) continue;
        bool different = previous < 0;
        if (!different) {
            int offset = 0;
            for (;;) {
                if (symbols[position + offset] != symbols[previous + offset] ||
                    small[position + offset] != small[previous + offset]) {
                    different = true; break;
                }
                // 種別フラグが一致済みなので、両方が同時に次のLMSへ達する。
                if (offset > 0 && (small[position + offset] & 2)) break;
                ++offset;
            }
        }
        if (different) ++name;
        reduced[selected_id[position / 2]] = name;
        previous = position;
    }
    selected_id.reset();
    const bool distinct_names = name + 1 == count;
    auto reduced_order = distinct_names ? std::vector<int>(count) : suffix_induced(std::span<const int>(reduced), name);
    if (distinct_names) {
        for (int i = 0; i < count; ++i) reduced_order[reduced[i]] = i;
    }
    for (int& position : reduced_order) position = selected[position];
    induce(reduced_order, true);
    return suffixes;
}

inline std::vector<int> suffix_string(std::string_view s) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_length_check(s.size());
    auto encoded = std::make_unique_for_overwrite<std::uint16_t[]>(s.size() + 1);
    encoded[s.size()] = 0;
    for (std::size_t i = 0; i < s.size(); ++i) encoded[i] = static_cast<std::uint16_t>(static_cast<unsigned char>(s[i]) + 1);
    auto result = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_induced(std::span<const std::uint16_t>(encoded.get(), s.size() + 1), 256);
    result.erase(result.begin());
    return result;
}

template <class Sequence, class Pattern, class Convert>
std::pair<int, int> suffix_search(const Sequence& s, const std::vector<int>& sa,
                                 const Pattern& pattern, Convert convert) {
    suffix_length_check(s.size());
    const auto compare = [&](int slot) {
        const int position = sa[slot];
        std::size_t offset = 0;
        const std::size_t remaining = s.size() - static_cast<std::size_t>(position);
        while (offset < pattern.size() && offset < remaining) {
            const auto a = convert(s[static_cast<std::size_t>(position) + offset]);
            const auto b = convert(pattern[offset]);
            if (a < b) return -1;
            if (b < a) return 1;
            ++offset;
        }
        return offset == pattern.size() ? 0 : -1;
    };
    int first = 0, last = static_cast<int>(sa.size());
    while (first < last) {
        const int middle = first + (last - first) / 2;
        if (compare(middle) < 0) first = middle + 1; else last = middle;
    }
    const int lower = first;
    last = static_cast<int>(sa.size());
    while (first < last) {
        const int middle = first + (last - first) / 2;
        if (compare(middle) <= 0) first = middle + 1; else last = middle;
    }
    return {lower, first};
}
}
#endif

#ifndef SIRKIT_SUFFIX_ARRAY_DEFINED
#define SIRKIT_SUFFIX_ARRAY_DEFINED
inline std::vector<int> suffix_array(const std::string& s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_string(s);
}
template<class View> requires std::same_as<View, std::string_view>
std::vector<int> suffix_array(View s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_string(s);
}
template<class Char> requires (std::same_as<Char, char> || std::same_as<Char, const char>)
std::vector<int> suffix_array(Char* s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_string(std::string_view(s));
}

inline std::vector<int> suffix_array(const std::vector<int>& s, int upper) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_length_check(s.size());
    if (upper < 0 || upper >= std::numeric_limits<int>::max() - 1)
        throw std::length_error("suffix_array: alphabet too large");
    std::vector<int> encoded(s.size() + 1);
    for (std::size_t i = 0; i < s.size(); ++i) {
        encoded[i] = s[i] + 1;
    }
    auto result = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_induced(std::span<const int>(encoded), upper + 1);
    result.erase(result.begin());
    return result;
}

template <class T>
std::vector<int> suffix_array(const std::vector<T>& s) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_length_check(s.size());
    // 先頭要素の同値類がn個なら、その順序だけで接尾辞順が決まる。
    if constexpr (std::integral<T>) {
        std::vector<T> alphabet = s;
        std::sort(alphabet.begin(), alphabet.end());
        alphabet.erase(std::unique(alphabet.begin(), alphabet.end()), alphabet.end());
        if (alphabet.size() == s.size()) {
            std::vector<int> result(s.size());
            for (std::size_t i = 0; i < s.size(); ++i)
                result[std::lower_bound(alphabet.begin(), alphabet.end(), s[i]) - alphabet.begin()] = static_cast<int>(i);
            return result;
        }
        std::vector<int> encoded(s.size() + 1);
        for (std::size_t i = 0; i < s.size(); ++i)
            encoded[i] = static_cast<int>(std::lower_bound(alphabet.begin(), alphabet.end(), s[i]) - alphabet.begin()) + 1;
        auto result = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_induced(std::span<const int>(encoded), static_cast<int>(alphabet.size()));
        result.erase(result.begin());
        return result;
    } else {
        std::vector<int> positions(s.size()), encoded(s.size() + 1);
        for (std::size_t i = 0; i < s.size(); ++i) positions[i] = static_cast<int>(i);
        std::sort(positions.begin(), positions.end(), [&](int a, int b) { return s[a] < s[b]; });
        int alphabet = 0;
        for (std::size_t i = 0; i < positions.size(); ++i) {
            if (i == 0 || s[positions[i - 1]] < s[positions[i]]) ++alphabet;
            encoded[positions[i]] = alphabet;
        }
        if (alphabet == static_cast<int>(s.size())) return positions;
        auto result = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_induced(std::span<const int>(encoded), alphabet);
        result.erase(result.begin());
        return result;
    }
}
#endif

#ifndef SIRKIT_LCP_ARRAY_IMPL_DEFINED
#define SIRKIT_LCP_ARRAY_IMPL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template <class Sequence>
std::vector<int> suffix_lcp(const Sequence& s, const std::vector<int>& sa) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_length_check(s.size());
    const int n = static_cast<int>(s.size());
    if (n == 0) return {};
    // 妥当なsaの順列から全要素を設定してから参照する。
    auto rank = std::make_unique_for_overwrite<int[]>(s.size());
    for (int i = 0; i < n; ++i) {
        rank[sa[i]] = i;
    }
    if (n < 2) return {};
    std::vector<int> lcp(static_cast<std::size_t>(n - 1));
    int length = 0;
    for (int i = 0; i < n; ++i) {
        const int order = rank[i];
        if (order + 1 == n) { length = 0; continue; }
        const int j = sa[order + 1];
        while (length < n - i && length < n - j && s[i + length] == s[j + length]) ++length;
        lcp[order] = length;
        if (length != 0) --length;
    }
    return lcp;
}
}
#endif

#ifndef SIRKIT_LCP_ARRAY_DEFINED
#define SIRKIT_LCP_ARRAY_DEFINED
template<class T>
std::vector<int> lcp_array(const std::vector<T>& s, const std::vector<int>& sa) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_lcp(s, sa);
}
inline std::vector<int> lcp_array(const std::string& s, const std::vector<int>& sa) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_lcp(s, sa);
}
template<class Sequence>
    requires requires(const Sequence& s) { s.size(); s[0] == s[0]; }
std::vector<int> lcp_array(const Sequence& s, const std::vector<int>& sa) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_lcp(s, sa);
}
#endif

#ifndef SIRKIT_SUFFIX_ARRAY_EQUAL_RANGE_DEFINED
#define SIRKIT_SUFFIX_ARRAY_EQUAL_RANGE_DEFINED
inline std::pair<int, int> SuffixArrayEqualRange(std::string_view s, const std::vector<int>& sa,
                                               std::string_view pattern) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_search(s, sa, pattern, [](char c) { return static_cast<unsigned char>(c); });
}
template <std::integral T>
std::pair<int, int> SuffixArrayEqualRange(const std::vector<T>& s, const std::vector<int>& sa,
                                       const std::vector<T>& pattern) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_search(s, sa, pattern, [](T value) { return value; });
}
#endif

#ifndef SIRKIT_STATIC_RANGE_MINIMUM_DEFINED
#define SIRKIT_STATIC_RANGE_MINIMUM_DEFINED

template<class T, class Compare = std::less<>>
class StaticRangeMinimum
{
    struct Data
    {
        std::vector<T> values;
        [[no_unique_address]] Compare compare;
        std::size_t block_width;
        std::vector<std::size_t> masks;
        std::vector<std::vector<std::size_t>> block_table;

        Data(std::vector<T> input, Compare order)
            : values(std::move(input)),
              compare(std::move(order)),
              block_width(std::max(std::size_t{1}, static_cast<std::size_t>(std::bit_width(values.size())))),
              masks(values.size())
        {
            std::vector<std::size_t> blocks;
            for (std::size_t first = 0; first < values.size();)
            {
                const auto end = first + std::min(block_width, values.size() - first);
                std::size_t stack = 0;
                for (std::size_t i = first; i < end; ++i)
                {
                    // 単調スタックに残った位置をビットで保存する。同値の位置は残す。
                    while (stack != 0)
                    {
                        const auto top = std::bit_width(stack) - 1;
                        if (!compare(values[i], values[first + top]))
                        {
                            break;
                        }
                        stack ^= std::size_t{1} << top;
                    }
                    stack |= std::size_t{1} << (i - first);
                    masks[i] = stack;
                }
                blocks.push_back(first + std::countr_zero(stack));
                first = end;
            }

            const auto count = blocks.size();
            block_table.push_back(std::move(blocks));
            for (std::size_t width = 2; width <= count; width *= 2)
            {
                const auto &previous = block_table.back();
                std::vector<std::size_t> row(count - width + 1);
                for (std::size_t i = 0; i < row.size(); ++i)
                {
                    row[i] = earlier_min(previous[i], previous[i + width / 2]);
                }
                block_table.push_back(std::move(row));
            }
        }

        std::size_t earlier_min(std::size_t left, std::size_t right) const
        {
            return compare(values[right], values[left]) ? right : left;
        }

        std::size_t within_block(std::size_t first, std::size_t left, std::size_t right) const
        {
            const auto masked = masks[right - 1] & (~std::size_t{0} << (left - first));
            return first + std::countr_zero(masked);
        }

        std::size_t argmin(std::size_t left, std::size_t right) const
        {
            const auto begin_block = left / block_width;
            const auto end_block = (right - 1) / block_width;
            const auto begin_pos = begin_block * block_width;
            if (begin_block == end_block)
            {
                return within_block(begin_pos, left, right);
            }
            const auto end_pos = end_block * block_width;
            auto answer = within_block(begin_pos, left, begin_pos + block_width);
            if (begin_block + 1 < end_block)
            {
                const auto count = end_block - (begin_block + 1);
                const auto level = std::bit_width(count) - 1;
                const auto &row = block_table[level];
                const auto middle = earlier_min(row[begin_block + 1], row[end_block - (std::size_t{1} << level)]);
                answer = earlier_min(answer, middle);
            }
            return earlier_min(answer, within_block(end_pos, end_pos, right));
        }
    };

    std::shared_ptr<const Data> data_;

public:
    using const_reference = typename std::vector<T>::const_reference;

    explicit StaticRangeMinimum(std::vector<T> values, Compare compare = Compare{})
        : data_(std::make_shared<const Data>(std::move(values), std::move(compare)))
    {
    }

    StaticRangeMinimum(const StaticRangeMinimum &) noexcept = default;
    StaticRangeMinimum &operator=(const StaticRangeMinimum &) noexcept = default;

    [[nodiscard]] std::size_t size() const noexcept
    {
        return data_->values.size();
    }

    [[nodiscard]] std::size_t argmin(std::size_t left, std::size_t right) const
    {
        return data_->argmin(left, right);
    }

    [[nodiscard]] const_reference min(std::size_t left, std::size_t right) const
    {
        return data_->values[argmin(left, right)];
    }
};

#endif

#ifndef SIRKIT_SUBSTRING_COMPARISON_DEFINED
#define SIRKIT_SUBSTRING_COMPARISON_DEFINED

class SubstringComparison
{
    struct Data
    {
        std::vector<int> suffix_rank;
        StaticRangeMinimum<int> adjacent_lcp;
    };

    std::shared_ptr<const Data> data_;

    static Data prepare(std::string_view text)
    {
        const auto suffixes = suffix_array(text);
        std::vector<int> rank(suffixes.size());
        for (std::size_t i = 0; i < suffixes.size(); ++i)
        {
            rank[suffixes[i]] = static_cast<int>(i);
        }
        return {std::move(rank), StaticRangeMinimum<int>(lcp_array(text, suffixes))};
    }

    void check_range(std::size_t first, std::size_t length) const
    {
        (void)first;
        (void)length;
    }

    std::size_t common_prefix_unchecked(std::size_t first, std::size_t second) const
    {
        if (first == second || first == size() || second == size())
        {
            return size() - std::max(first, second);
        }
        const auto [left, right] = std::minmax(data_->suffix_rank[first], data_->suffix_rank[second]);
        return static_cast<std::size_t>(data_->adjacent_lcp.min(static_cast<std::size_t>(left),
                                                               static_cast<std::size_t>(right)));
    }

public:
    explicit SubstringComparison(std::string_view text) : data_(std::make_shared<const Data>(prepare(text)))
    {
    }

    SubstringComparison(const SubstringComparison &) noexcept = default;
    SubstringComparison &operator=(const SubstringComparison &) noexcept = default;

    [[nodiscard]] std::size_t size() const noexcept
    {
        return data_->suffix_rank.size();
    }

    [[nodiscard]] std::size_t common_prefix(std::size_t first, std::size_t second) const
    {
        check_range(first, 0);
        check_range(second, 0);
        return common_prefix_unchecked(first, second);
    }

    [[nodiscard]] bool equal(std::size_t first, std::size_t second, std::size_t length) const
    {
        check_range(first, length);
        check_range(second, length);
        return common_prefix_unchecked(first, second) >= length;
    }

    [[nodiscard]] std::strong_ordering compare(std::size_t first, std::size_t first_length,
                                               std::size_t second, std::size_t second_length) const
    {
        check_range(first, first_length);
        check_range(second, second_length);
        if (common_prefix_unchecked(first, second) >= std::min(first_length, second_length))
        {
            return first_length <=> second_length;
        }
        return data_->suffix_rank[first] <=> data_->suffix_rank[second];
    }
};

#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <algorithm>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <array>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <compare>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <numeric>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <optional>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <span>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <string>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <string_view>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

#ifndef SIRKIT_LEXICOGRAPHIC_CONCATENATION_DEFINED
#define SIRKIT_LEXICOGRAPHIC_CONCATENATION_DEFINED

namespace detail
{
inline std::size_t ExtendConcatenationLength(std::size_t length, std::size_t word)
{
    return length + word;
}

struct ConcatenationSlice
{
    std::size_t first, end;
};

inline std::strong_ordering CompareConcatenations(const SubstringComparison &index,
                                                  ConcatenationSlice first, ConcatenationSlice second)
{
    // 空文字列を並べ替えの先頭に集め、比較関係を狭義弱順序にする。
    const bool empty_first = first.first == first.end, empty_second = second.first == second.end;
    if (empty_first || empty_second)
    {
        return empty_second <=> empty_first;
    }
    std::array lhs{first, second};
    std::array rhs{second, first};
    for (std::size_t i = 0, j = 0; i < lhs.size();)
    {
        auto &a = lhs[i];
        auto &b = rhs[j];
        const auto length = std::min(a.end - a.first, b.end - b.first);
        const auto compare = index.compare(a.first, length, b.first, length);
        if (compare != 0)
        {
            return compare;
        }
        a.first += length;
        b.first += length;
        i += a.first == a.end;
        j += b.first == b.end;
    }
    return std::strong_ordering::equal;
}

struct ConcatenationOrder
{
    std::vector<std::size_t> indices;
    std::size_t length = 0;
    bool tied = false;
};

inline ConcatenationOrder BuildConcatenationOrder(std::span<const std::string_view> words)
{
    ConcatenationOrder result;
    result.indices.resize(words.size());
    std::iota(result.indices.begin(), result.indices.end(), std::size_t{0});
    for (const auto word : words)
    {
        result.length = ExtendConcatenationLength(result.length, word.size());
    }
    if (words.size() < 2)
    {
        return result;
    }

    std::string text;
    text.reserve(result.length);
    std::vector<ConcatenationSlice> slices;
    slices.reserve(words.size());
    for (const auto word : words)
    {
        const auto start = text.size();
        text.append(word);
        slices.push_back({start, text.size()});
        result.tied |= word.empty();
    }
    const SubstringComparison index{std::string_view(text)};
    const auto compare = [&](std::size_t a, std::size_t b)
    {
        return CompareConcatenations(index, slices[a], slices[b]);
    };
    std::sort(result.indices.begin(), result.indices.end(),
              [&](std::size_t a, std::size_t b)
              {
                  return compare(a, b) < 0;
              });
    if (!result.tied)
    {
        result.tied = std::adjacent_find(result.indices.begin(), result.indices.end(),
                                         [&](std::size_t a, std::size_t b)
                                         {
                                             return compare(a, b) == 0;
                                         }) != result.indices.end();
    }
    return result;
}
}

[[nodiscard]] inline std::vector<std::size_t> lexicographic_concatenation_order(
    std::span<const std::string_view> words)
{
    return detail::BuildConcatenationOrder(words).indices;
}

[[nodiscard]] inline std::optional<std::string> second_lexicographic_concatenation(
    std::span<const std::string_view> words)
{
    if (words.size() < 2)
    {
        return std::nullopt;
    }
    if (words.size() == 2)
    {
        const auto size = detail::ExtendConcatenationLength(detail::ExtendConcatenationLength(0, words[0].size()),
                                                            words[1].size());
        std::string first, second;
        first.reserve(size);
        second.reserve(size);
        first.append(words[0]).append(words[1]);
        second.append(words[1]).append(words[0]);
        return first < second ? std::move(second) : std::move(first);
    }

    const auto [indices, length, tied] = detail::BuildConcatenationOrder(words);
    const auto n = indices.size();
    std::string result;
    result.reserve(length);
    if (tied)
    {
        for (const auto i : indices)
        {
            result.append(words[i]);
        }
        return result;
    }

    // 離れた2箇所の隣接交換は、後ろの箇所で交換した候補の方が小さい。
    // 重複がなければ辞書順2番目は末尾の2種類の隣接交換のどちらか。
    for (std::size_t i = 0; i < n - 3; ++i)
    {
        result.append(words[indices[i]]);
    }
    const auto a = words[indices[n - 3]], b = words[indices[n - 2]], c = words[indices[n - 1]];
    std::string tail_first, tail_second;
    const auto tail_size = a.size() + b.size() + c.size();
    tail_first.reserve(tail_size);
    tail_second.reserve(tail_size);
    tail_first.append(a).append(c).append(b);
    tail_second.append(b).append(a).append(c);
    result.append(std::min(tail_first, tail_second));
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
