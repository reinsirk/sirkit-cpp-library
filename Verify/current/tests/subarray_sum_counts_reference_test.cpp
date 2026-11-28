#include "../../../Algorithm/Subarray_Sum_Counts.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
using sirkit::SubarraySumBounds;
using sirkit::count_subarrays_by_sum;

std::uint64_t brute(std::span<const std::int64_t> values, const SubarraySumBounds &bounds)
{
    std::uint64_t answer = 0;
    for (std::size_t left = 0; left < values.size(); ++left)
    {
        std::int64_t sum = 0;
        for (std::size_t right = left; right < values.size(); ++right)
        {
            sum += values[right];
            const std::size_t length = right - left + 1;
            if (length < bounds.minimum_length || length > bounds.maximum_length)
            {
                continue;
            }
            if (bounds.minimum_sum && sum < *bounds.minimum_sum)
            {
                continue;
            }
            if (bounds.maximum_sum && sum > *bounds.maximum_sum)
            {
                continue;
            }
            ++answer;
        }
    }
    return answer;
}

template <typename Exception, typename Action>
void expect_exception(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}
}

int main()
{
    assert(count_subarrays_by_sum({}) == 0);
    const std::array<std::int64_t, 4> zeros{};
    assert(count_subarrays_by_sum(zeros) == 10);
    assert(count_subarrays_by_sum(zeros, {.minimum_sum = 0, .maximum_sum = 0, .minimum_length = 2}) == 6);
    assert(count_subarrays_by_sum(zeros, {.maximum_sum = -1}) == 0);

    std::mt19937 random(233196);
    for (int test = 0; test < 2000; ++test)
    {
        const auto n = static_cast<std::size_t>(random() % 37);
        std::vector<std::int64_t> values(n);
        for (auto &value : values)
        {
            value = static_cast<std::int64_t>(random() % 21) - (test % 2 ? 10 : 0);
        }
        for (int limits = 0; limits < 5; ++limits)
        {
            auto x = static_cast<std::int64_t>(random() % 121) - 60;
            auto y = static_cast<std::int64_t>(random() % 121) - 60;
            if (limits == 4)
            {
                y = x;
            }
            const auto a = static_cast<std::size_t>(random() % (n + 3)) + 1;
            const auto b = static_cast<std::size_t>(random() % (n + 3)) + 1;
            SubarraySumBounds bounds{
                .minimum_length = std::min(a, b),
                .maximum_length = std::max(a, b),
            };
            if ((limits & 1) || limits >= 3)
            {
                bounds.minimum_sum = std::min(x, y);
            }
            if ((limits & 2) || limits >= 3)
            {
                bounds.maximum_sum = std::max(x, y);
            }
            assert(count_subarrays_by_sum(values, bounds) == brute(values, bounds));
        }
    }

    constexpr auto low = std::numeric_limits<std::int64_t>::min();
    constexpr auto high = std::numeric_limits<std::int64_t>::max();
    const std::array above{low, high, high};
    assert(count_subarrays_by_sum(above, {.minimum_sum = high}) == 3);
    assert(count_subarrays_by_sum(above, {.minimum_sum = high, .maximum_sum = high}) == 2);
    const std::array below{high, low, low + 1};
    assert(count_subarrays_by_sum(below, {.maximum_sum = low}) == 3);
    assert(count_subarrays_by_sum(below, {.minimum_sum = low, .maximum_sum = low}) == 2);

    expect_exception<std::length_error>([] { sirkit::detail::CheckSubarraySumCountSize(std::numeric_limits<int>::max()); });
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::invalid_argument>([&] { (void)count_subarrays_by_sum(zeros, {.minimum_length = 0}); });
    expect_exception<std::invalid_argument>([&] { (void)count_subarrays_by_sum(zeros, {.minimum_length = 3, .maximum_length = 2}); });
    expect_exception<std::invalid_argument>([&] { (void)count_subarrays_by_sum(zeros, {.minimum_sum = 3, .maximum_sum = 2}); });
    expect_exception<std::overflow_error>([&] { (void)count_subarrays_by_sum(std::array{high, std::int64_t{1}}); });
    expect_exception<std::overflow_error>([&] { (void)count_subarrays_by_sum(std::array{low, std::int64_t{-1}}); });
#endif
}
