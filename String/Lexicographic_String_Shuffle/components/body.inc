#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifndef SIRKIT_LEXICOGRAPHIC_STRING_SHUFFLE_DEFINED
#define SIRKIT_LEXICOGRAPHIC_STRING_SHUFFLE_DEFINED

namespace detail
{
inline std::size_t ExtendShuffleLength(std::size_t used, std::size_t length)
{
#ifndef SKIP_RUNTIME_CHECKS
    constexpr auto kLimit = static_cast<std::size_t>(std::numeric_limits<int>::max()) - 2;
    if (length >= kLimit - used)
    {
        throw std::length_error("lexicographic_string_shuffle: input too long");
    }
#endif
    return used + length + 1;
}
}

[[nodiscard]] inline std::string lexicographic_string_shuffle(std::span<const std::string_view> words)
{
    std::size_t joined_size = 0, active = 0;
    std::string_view only;
    for (const auto word : words)
    {
        if (word.empty())
        {
            continue;
        }
        joined_size = detail::ExtendShuffleLength(joined_size, word.size());
        only = word;
        ++active;
    }
    if (active <= 1)
    {
        return std::string(only);
    }

    struct RemainingWord
    {
        int first, end;
    };
    constexpr int kTerminator = 256;
    std::vector<int> symbols;
    std::vector<RemainingWord> candidates;
    symbols.reserve(joined_size);
    candidates.reserve(active);
    for (const auto word : words)
    {
        if (word.empty())
        {
            continue;
        }
        const auto first = static_cast<int>(symbols.size());
        for (const unsigned char byte : word)
        {
            symbols.push_back(byte);
        }
        candidates.push_back({first, static_cast<int>(symbols.size())});
        // 同じ接尾辞の順序は任意。異なる接尾辞は最初の終端までで順序が確定する。
        symbols.push_back(kTerminator);
    }

    std::vector<int> rank(joined_size);
    const auto order = suffix_array(symbols, kTerminator);
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        rank[order[i]] = static_cast<int>(i);
    }
    const auto lower_priority = [&](RemainingWord a, RemainingWord b)
    {
        return rank[a.first] > rank[b.first];
    };
    std::make_heap(candidates.begin(), candidates.end(), lower_priority);
    std::string merged;
    merged.reserve(joined_size - active);
    while (!candidates.empty())
    {
        std::pop_heap(candidates.begin(), candidates.end(), lower_priority);
        auto next = candidates.back();
        candidates.pop_back();
        merged.push_back(static_cast<char>(symbols[next.first++]));
        if (next.first != next.end)
        {
            candidates.push_back(next);
            std::push_heap(candidates.begin(), candidates.end(), lower_priority);
        }
    }
    return merged;
}

#endif
