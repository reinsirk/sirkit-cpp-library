#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <stdexcept>

#ifndef SIRKIT_SUBARRAY_FREQUENCY_WINDOWS_DEFINED
#define SIRKIT_SUBARRAY_FREQUENCY_WINDOWS_DEFINED

struct SubarrayFrequencyBounds
{
    std::size_t maximum_distinct = std::numeric_limits<std::size_t>::max();
    std::size_t maximum_occurrences = std::numeric_limits<std::size_t>::max();
};

struct SubarrayFrequencyResult
{
    std::uint64_t count = 0;
    std::size_t left = 0;
    std::size_t right = 0;

    [[nodiscard]] std::size_t length() const noexcept
    {
        return right - left;
    }
};

namespace reinsirk::detail
{
inline std::uint64_t IncreaseFrequencyWindowCount(std::uint64_t count, std::size_t amount)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (amount > std::numeric_limits<std::uint64_t>::max() - count)
    {
        throw std::overflow_error("analyze_frequency_bounded_subarrays: count overflow");
    }
#endif
    return count + amount;
}
} // namespace reinsirk::detail

template <typename Range>
    requires std::ranges::forward_range<const Range> &&
             std::integral<std::ranges::range_value_t<const Range>> &&
             (sizeof(std::ranges::range_value_t<const Range>) <= sizeof(std::uint64_t))
[[nodiscard]] SubarrayFrequencyResult analyze_frequency_bounded_subarrays(const Range &values, SubarrayFrequencyBounds bounds = {})
{
    SubarrayFrequencyResult result;
    if (bounds.maximum_distinct == 0 || bounds.maximum_occurrences == 0)
    {
        return result;
    }

    HashMap<std::uint64_t, std::size_t> frequencies;
    auto left_value = std::ranges::begin(values);
    std::size_t left = 0, right = 0;
    for (const auto value : values)
    {
        const auto incoming = static_cast<std::uint64_t>(value);
        std::size_t occurrences = ++frequencies[incoming];
        ++right;

        // 追加前の区間は両上限を満たす。出現回数を超える可能性があるのは追加した値だけ。
        while (frequencies.size() > bounds.maximum_distinct || occurrences > bounds.maximum_occurrences)
        {
            const auto outgoing = static_cast<std::uint64_t>(*left_value);
            auto position = frequencies.find(outgoing);
            if (--position->second == 0)
            {
                frequencies.erase(position, false);
            }
            if (outgoing == incoming)
            {
                --occurrences;
            }
            ++left_value;
            ++left;
        }

        const std::size_t length = right - left;
        result.count = reinsirk::detail::IncreaseFrequencyWindowCount(result.count, length);
        if (length > result.length())
        {
            result.left = left;
            result.right = right;
        }
    }
    return result;
}

#endif
