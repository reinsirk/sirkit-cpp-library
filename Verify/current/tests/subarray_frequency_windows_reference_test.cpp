#include "../../../Algorithm/Subarray_Frequency_Windows.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <forward_list>
#include <limits>
#include <map>
#include <random>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace
{
using sirkit::SubarrayFrequencyBounds;
using sirkit::SubarrayFrequencyResult;
using sirkit::analyze_frequency_bounded_subarrays;

SubarrayFrequencyResult brute(std::span<const std::int64_t> values, SubarrayFrequencyBounds bounds)
{
    SubarrayFrequencyResult result;
    for (std::size_t left = 0; left < values.size(); ++left)
    {
        std::map<std::int64_t, std::size_t> frequencies;
        for (std::size_t right = left; right < values.size(); ++right)
        {
            const auto frequency = ++frequencies[values[right]];
            if (frequencies.size() > bounds.maximum_distinct || frequency > bounds.maximum_occurrences)
            {
                break;
            }
            ++result.count;
            if (right + 1 - left > result.length())
            {
                result.left = left;
                result.right = right + 1;
            }
        }
    }
    return result;
}
}

int main()
{
    const auto empty = analyze_frequency_bounded_subarrays(std::array<int, 0>{});
    assert(empty.count == 0 && empty.left == 0 && empty.right == 0);

    const std::forward_list<int> list{-2, 1, -2, 3, 1};
    const auto list_result = analyze_frequency_bounded_subarrays(list, {.maximum_occurrences = 1});
    assert(list_result.count == 11 && list_result.left == 1 && list_result.right == 4);

    const std::string_view bytes{"a\0ab", 4};
    const auto byte_result = analyze_frequency_bounded_subarrays(bytes, {.maximum_occurrences = 1});
    assert(byte_result.count == 8 && byte_result.left == 1 && byte_result.right == 4);

    constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
    const std::array<std::uint64_t, 4> extreme{maximum, 0, maximum - 1, maximum};
    const auto extreme_result = analyze_frequency_bounded_subarrays(extreme, {.maximum_occurrences = 1});
    assert(extreme_result.count == 9 && extreme_result.left == 0 && extreme_result.right == 3);

    std::mt19937 random(34222);
    for (int trial = 0; trial < 2600; ++trial)
    {
        const auto n = static_cast<std::size_t>(random() % 48);
        const auto alphabet = static_cast<std::int64_t>(random() % 40) + 1;
        std::vector<std::int64_t> values(n);
        for (auto &value : values)
        {
            value = static_cast<std::int64_t>(random() % alphabet) - 20;
        }
        std::array<SubarrayFrequencyBounds, 4> bounds{
            SubarrayFrequencyBounds{},
            SubarrayFrequencyBounds{.maximum_distinct = static_cast<std::size_t>(random() % 15)},
            SubarrayFrequencyBounds{.maximum_occurrences = static_cast<std::size_t>(random() % 11)},
            SubarrayFrequencyBounds{static_cast<std::size_t>(random() % 15), static_cast<std::size_t>(random() % 11)},
        };
        for (const auto bound : bounds)
        {
            const auto expected = brute(values, bound);
            const auto actual = analyze_frequency_bounded_subarrays(values, bound);
            assert(actual.count == expected.count && actual.left == expected.left && actual.right == expected.right);
        }
    }

    const std::vector<int> repeated(100000, 7);
    const auto full = analyze_frequency_bounded_subarrays(repeated, {.maximum_distinct = 1});
    assert(full.count == 5000050000ULL && full.left == 0 && full.right == repeated.size());
#ifndef SKIP_RUNTIME_CHECKS
    bool overflow = false;
    try
    {
        (void)sirkit::detail::IncreaseFrequencyWindowCount(maximum - 1, 2);
    }
    catch (const std::overflow_error &)
    {
        overflow = true;
    }
    assert(overflow);
#endif
}
