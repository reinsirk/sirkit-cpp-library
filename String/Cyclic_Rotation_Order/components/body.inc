#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifndef SIRKIT_CYCLIC_ROTATION_ORDER_DEFINED
#define SIRKIT_CYCLIC_ROTATION_ORDER_DEFINED

namespace reinsirk::detail
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

    const auto count = ::reinsirk::detail::checked_rotation_count(length, texts.size());
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
