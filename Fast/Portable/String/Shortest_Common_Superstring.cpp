// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Shortest_Common_Superstring.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_common_superstring.md
//
// API一覧
// shortest_common_superstring(words): 各要素を連続部分文字列として含む最短の文字列を一つ返す。
// 空の要素、重複、他の要素に含まれる要素を除く。入力20個以下、全長の合計はUINT32_MAX以下。
// Nは入力数、Sは合計長、Kは除外後の要素数。O(N*S+K^2*2^K+1)時間・O(S+K*2^K+N^2+1)領域。
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
#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>
#include <span>
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

#ifndef SIRKIT_SHORTEST_COMMON_SUPERSTRING_DEFINED
#define SIRKIT_SHORTEST_COMMON_SUPERSTRING_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::uint32_t superstring_add_length(std::uint32_t total, std::size_t length)
{
    if (length > std::numeric_limits<std::uint32_t>::max() - total)
    {
        throw std::length_error("shortest_common_superstring: total length exceeds UINT32_MAX");
    }
    return total + static_cast<std::uint32_t>(length);
}

inline std::size_t superstring_overlap(std::string_view text, std::string_view pattern,
                                       const std::vector<std::size_t> &border)
{
    std::size_t matched = 0;
    for (const char byte : text)
    {
        while (matched != 0 && byte != pattern[matched])
        {
            matched = border[matched - 1];
        }
        matched += byte == pattern[matched];
        if (matched == pattern.size())
        {
            // patternを含む場合の値。そのpatternを残すときはこの辺を使わない。
            return matched;
        }
    }
    return matched;
}
} // namespace detail

inline std::string shortest_common_superstring(std::span<const std::string_view> words)
{
    constexpr std::size_t kMaxWords = 20;
    if (words.size() > kMaxWords)
    {
        throw std::length_error("shortest_common_superstring: too many words");
    }
    std::uint32_t total_length = 0;
    std::vector<std::string_view> candidates;
    for (const auto word : words)
    {
        total_length = detail::superstring_add_length(total_length, word.size());
        if (!word.empty())
        {
            candidates.push_back(word);
        }
    }
    if (candidates.empty())
    {
        return {};
    }

    const auto count = candidates.size();
    std::vector<std::uint32_t> overlaps(count * count);
    std::vector<std::size_t> retained;
    std::size_t retained_length = 0;
    for (std::size_t second = 0; second < count; ++second)
    {
        const auto word = candidates[second];
        const auto border = prefix_function(word);
        bool contained = false;
        for (std::size_t first = 0; first < count; ++first)
        {
            if (first == second)
            {
                continue;
            }
            const auto text = candidates[first];
            const auto common = detail::superstring_overlap(text, word, border);
            overlaps[first * count + second] = static_cast<std::uint32_t>(common);
            if (common == word.size() && (text.size() > word.size() || first < second))
            {
                contained = true;
                break;
            }
        }
        if (!contained)
        {
            retained.push_back(second);
            retained_length += word.size();
        }
    }

    const auto k = retained.size();
    const auto state_count = std::size_t{1} << k;
    const auto per_last = state_count / 2;
    std::vector<std::uint32_t> best(k * per_last);
    const auto score = [&](std::size_t mask, std::size_t last) -> std::uint32_t & {
        // lastのビットは必ず1なので、省いた添字に詰めて保存する。
        const auto low_bits = (std::size_t{1} << last) - 1;
        return best[last * per_last + (mask & low_bits) + ((mask >> 1) & ~low_bits)];
    };
    const auto link = [&](std::size_t first, std::size_t second) {
        return overlaps[retained[first] * count + retained[second]];
    };
    for (std::size_t mask = 1; mask < state_count; ++mask)
    {
        for (auto bits = mask; bits != 0; bits &= bits - 1)
        {
            const auto last = static_cast<std::size_t>(std::countr_zero(bits));
            const auto smaller = mask ^ (std::size_t{1} << last);
            auto &result = score(mask, last);
            for (auto predecessors = smaller; predecessors != 0; predecessors &= predecessors - 1)
            {
                const auto previous = static_cast<std::size_t>(std::countr_zero(predecessors));
                result = std::max(result, score(smaller, previous) + link(previous, last));
            }
        }
    }

    const auto full = state_count - 1;
    std::size_t last = 0;
    for (std::size_t i = 1; i < k; ++i)
    {
        if (score(full, i) > score(full, last))
        {
            last = i;
        }
    }
    const auto answer_length = retained_length - score(full, last);
    std::vector<std::size_t> order;
    for (auto mask = full; mask != 0;)
    {
        order.push_back(last);
        const auto value = score(mask, last);
        mask ^= std::size_t{1} << last;
        for (auto bits = mask; bits != 0; bits &= bits - 1)
        {
            const auto previous = static_cast<std::size_t>(std::countr_zero(bits));
            if (score(mask, previous) + link(previous, last) == value)
            {
                last = previous;
                break;
            }
        }
    }

    std::string result;
    result.reserve(answer_length);
    result += candidates[retained[order.back()]];
    for (std::size_t index = order.size() - 1; index != 0; --index)
    {
        const auto next = order[index - 1];
        result += candidates[retained[next]].substr(link(order[index], next));
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
