#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifndef SIRKIT_LEXICOGRAPHIC_CONCATENATION_DEFINED
#define SIRKIT_LEXICOGRAPHIC_CONCATENATION_DEFINED

namespace detail
{
inline std::size_t ExtendConcatenationLength(std::size_t length, std::size_t word)
{
#ifndef SKIP_RUNTIME_CHECKS
    constexpr auto kLimit = static_cast<std::size_t>(std::numeric_limits<int>::max()) - 2;
    if (word > kLimit - length)
    {
        throw std::length_error("lexicographic_concatenation_order: input too long");
    }
#endif
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
