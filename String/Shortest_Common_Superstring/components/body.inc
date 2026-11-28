#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>
#include <span>
#include <string>

#ifndef SIRKIT_SHORTEST_COMMON_SUPERSTRING_DEFINED
#define SIRKIT_SHORTEST_COMMON_SUPERSTRING_DEFINED

namespace reinsirk::detail
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
