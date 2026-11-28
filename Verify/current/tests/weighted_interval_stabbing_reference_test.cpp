#include "../../../Algorithm/Weighted_Interval_Stabbing.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
using sirkit::StabbingInterval;
using sirkit::maximize_interval_stabbing_weight;

std::int64_t score(unsigned mask, std::span<const StabbingInterval> intervals)
{
    std::int64_t total = 0;
    for (const auto &[left, right, weight] : intervals)
    {
        for (int point = left; point < right; ++point)
        {
            if ((mask >> point) & 1U)
            {
                total += weight;
                break;
            }
        }
    }
    return total;
}

void verify(int size, std::span<const StabbingInterval> intervals)
{
    const auto result = maximize_interval_stabbing_weight(size, intervals);
    std::int64_t expected = 0;
    for (unsigned mask = 0; mask < (1U << size); ++mask)
    {
        expected = std::max(expected, score(mask, intervals));
    }
    unsigned mask = 0;
    int previous = -1;
    for (const auto point : result.points)
    {
        assert(previous < point && point < size);
        mask |= 1U << point;
        previous = point;
    }
    assert(result.weight == expected && score(mask, intervals) == expected);
}

#ifndef SKIP_RUNTIME_CHECKS
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
#endif
}

int main()
{
    using sirkit::detail::ApplyStabbingWeight;
    using sirkit::detail::BestStabbingState;
    using sirkit::detail::StabbingState;
    const StabbingState first{{42, 1}};
    const StabbingState second{{42, 2}};
    assert(BestStabbingState(std::nullopt, first) == first);
    assert(BestStabbingState(second, first) == first);
    assert(!ApplyStabbingWeight(123, std::nullopt));

    verify(0, {});
    verify(0, std::array{StabbingInterval{0, 0, 100}});
    verify(5, std::array{StabbingInterval{0, 3, 10}, StabbingInterval{1, 4, -10}, StabbingInterval{2, 5, 10}});
    verify(6, std::array{StabbingInterval{0, 6, -10}, StabbingInterval{1, 3, -2}, StabbingInterval{0, 6, 3}});

    std::mt19937 random(31703);
    for (int test = 0; test < 900; ++test)
    {
        const int size = static_cast<int>(random() % 12);
        const int count = static_cast<int>(random() % 27);
        std::vector<StabbingInterval> intervals;
        for (int i = 0; i < count; ++i)
        {
            const int left = static_cast<int>(random() % (size + 1));
            const int right = static_cast<int>(random() % (size + 1));
            const int weight = static_cast<int>(random() % 21) - 10;
            intervals.push_back({std::min(left, right), std::max(left, right), weight});
        }
        verify(size, intervals);
    }

    constexpr auto low = std::numeric_limits<std::int64_t>::min();
    constexpr auto high = std::numeric_limits<std::int64_t>::max();
    verify(1, std::array{StabbingInterval{0, 1, low}});
    verify(1, std::array{StabbingInterval{0, 1, high}});
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::invalid_argument>([] { (void)maximize_interval_stabbing_weight(-1, {}); });
    expect_exception<std::length_error>([] { (void)maximize_interval_stabbing_weight(std::numeric_limits<int>::max(), {}); });
    for (const auto interval : std::array{StabbingInterval{-1, 0, 1}, StabbingInterval{1, 0, 1}, StabbingInterval{0, 2, 1}})
    {
        expect_exception<std::invalid_argument>([&] { (void)maximize_interval_stabbing_weight(1, std::span{&interval, 1}); });
    }
    expect_exception<std::overflow_error>([&] {
        (void)maximize_interval_stabbing_weight(1, std::array{StabbingInterval{0, 1, high}, StabbingInterval{0, 1, 1}});
    });
    expect_exception<std::overflow_error>([&] {
        (void)maximize_interval_stabbing_weight(1, std::array{StabbingInterval{0, 1, low}, StabbingInterval{0, 1, -1}});
    });
#endif
}
