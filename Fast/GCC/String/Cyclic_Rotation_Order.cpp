// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Cyclic_Rotation_Order.cpp
// 参考（接尾辞配列による回転対の数え上げ）: https://atcoder.jp/contests/abc272/editorial/4988
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/cyclic_rotation_order.md
//
// API一覧
// cyclic_rotation_ranks(text): 各巡回シフトの辞書順順位。0始まりで同じ回転は同順位、欠番なし。
// cyclic_rotation_ranks(texts): 同じ長さnのm個の文字列を一緒に順位付け。結果[i*n+j]は第i文字列の開始位置j。
// cyclic_rotation_order(text): 開始位置を回転の辞書順に並べる。同順位では開始位置の小さい順。
// count_cyclic_rotation_pairs(first,second): 同じ長さの2列についてfirstの回転<=secondの回転となる開始位置対の個数。
// 符号なしバイト順。空文字には開始位置がなく、結果は空または0。2*m*n<=INT_MAX-2。
// 複数列の順位はO(m*n+m+1)時間・O(m*n+1)領域。他はO(n+1)時間・領域。
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
#include <array>
#include <numeric>


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

#ifndef SIRKIT_CYCLIC_ROTATION_ORDER_DEFINED
#define SIRKIT_CYCLIC_ROTATION_ORDER_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t checked_rotation_count(std::size_t width, std::size_t words)
{
    constexpr auto limit = static_cast<std::size_t>(std::numeric_limits<int>::max() - 2);
    if (words > limit / 2 / width)
    {
        throw std::length_error("cyclic_rotation_ranks: input too long");
    }
    return words * width;
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::vector<int> cyclic_rotation_ranks(std::span<const std::string_view> texts)
{
    if (texts.empty())
    {
        return {};
    }
    const auto length = texts.front().size();
    for (const auto text : texts)
    {
        if (text.size() != length)
        {
            throw std::invalid_argument("cyclic_rotation_ranks: unequal lengths");
        }
    }
    if (length == 0)
    {
        return {};
    }

    const auto count = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::checked_rotation_count(length, texts.size());
    const auto stride = 2 * length;
    std::string joined;
    joined.reserve(2 * count);
    for (const auto text : texts)
    {
        joined.append(text);
        joined.append(text);
    }
    const auto suffixes = suffix_array(joined);
    const auto adjacent_lcp = lcp_array(joined, suffixes);

    std::vector<int> ranks(count);
    int rank = 0;
    bool seen_rotation = false;
    // 長さlengthの接頭辞が同じ接尾辞を一つの順位にまとめる。
    for (std::size_t slot = 0; slot < suffixes.size(); ++slot)
    {
        if (slot == 0 || adjacent_lcp[slot - 1] < static_cast<int>(length))
        {
            rank += seen_rotation;
            seen_rotation = false;
        }
        const auto position = static_cast<std::size_t>(suffixes[slot]);
        const auto offset = position % stride;
        if (offset < length)
        {
            ranks[position / stride * length + offset] = rank;
            seen_rotation = true;
        }
    }
    return ranks;
}

[[nodiscard]] inline std::vector<int> cyclic_rotation_ranks(std::string_view text)
{
    return cyclic_rotation_ranks(std::span<const std::string_view>(&text, 1));
}

[[nodiscard]] inline std::vector<int> cyclic_rotation_order(std::string_view text)
{
    const auto ranks = cyclic_rotation_ranks(text);
    std::vector<int> order(ranks.size());
    std::vector<std::size_t> next(ranks.size() + 1);
    for (const auto rank : ranks)
    {
        ++next[static_cast<std::size_t>(rank) + 1];
    }
    std::partial_sum(next.begin(), next.end(), next.begin());
    for (std::size_t i = 0; i < ranks.size(); ++i)
    {
        order[next[static_cast<std::size_t>(ranks[i])]++] = static_cast<int>(i);
    }
    return order;
}

[[nodiscard]] inline std::uint64_t count_cyclic_rotation_pairs(std::string_view first, std::string_view second)
{
    const auto ranks = cyclic_rotation_ranks(std::array{first, second});
    std::vector<std::uint32_t> counts(ranks.size());
    for (std::size_t i = 0; i < first.size(); ++i)
    {
        ++counts[static_cast<std::size_t>(ranks[i])];
    }
    std::partial_sum(counts.begin(), counts.end(), counts.begin());
    std::uint64_t result = 0;
    for (std::size_t i = first.size(); i < ranks.size(); ++i)
    {
        result += counts[static_cast<std::size_t>(ranks[i])];
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
