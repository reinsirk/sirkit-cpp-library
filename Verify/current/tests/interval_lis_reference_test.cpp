// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_INTERVAL_LIS_DEFINED
#include "../../../Algorithm/Interval_LIS.cpp"
#endif
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
std::vector<std::int64_t> brute(std::span<const sirkit::LISInterval> intervals)
{
    std::vector<std::optional<std::int64_t>> least(intervals.size());
    std::size_t processed = 0;
    for (const auto &[left, right] : intervals)
    {
        ++processed;
        for (auto length = processed; length > 0; --length)
        {
            auto candidate = left;
            if (length > 1)
            {
                const auto previous = least[length - 2];
                if (!previous || *previous >= right)
                {
                    continue;
                }
                candidate = std::max(left, *previous + 1);
            }
            auto &selected = least[length - 1];
            if (!selected || candidate < *selected)
            {
                selected = candidate;
            }
        }
    }
    std::vector<std::int64_t> result;
    for (const auto candidate : least)
    {
        if (candidate)
        {
            result.push_back(*candidate);
        }
    }
    return result;
}

void check(std::span<const sirkit::LISInterval> intervals)
{
    assert(sirkit::interval_lis_tails(intervals) == brute(intervals));
}
} // namespace

int main()
{
    using Interval = sirkit::LISInterval;
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    check({});
    check(std::array{Interval{1, 1}});
    check(std::array{Interval{3, 5}, Interval{1, 3}, Interval{2, 7}, Interval{1, 2}, Interval{2, 3}});
    check(std::array{Interval{kMax, kMax}, Interval{kMax, kMax}, Interval{kMin, kMax}, Interval{kMax, kMax}});
    check(std::array{Interval{kMax - 2, kMax - 2}, Interval{kMax - 2, kMax}, Interval{kMax - 2, kMax},
                     Interval{kMax - 2, kMax}, Interval{kMax - 2, kMax}});
    check(std::array{Interval{kMin, kMin}, Interval{kMin, kMin + 2}, Interval{kMin, kMin + 2},
                     Interval{kMin, kMin + 2}, Interval{kMin, kMin + 2}});

    std::mt19937 random(38051);
    for (int trial = 0; trial < 1200; ++trial)
    {
        std::vector<Interval> intervals(random() % 73);
        for (auto &[left, right] : intervals)
        {
            left = static_cast<std::int64_t>(random() % 41) - 20;
            right = left + static_cast<std::int64_t>(random() % 21);
        }
        check(intervals);
    }

    constexpr std::size_t kSize = 100'000;
    const auto large = sirkit::interval_lis_tails(std::vector<Interval>(kSize, {kMin, kMax}));
    assert(large.size() == kSize);
    for (std::size_t i = 0; i < kSize; ++i)
    {
        assert(large[i] == kMin + static_cast<std::int64_t>(i));
    }
#ifndef SKIP_RUNTIME_CHECKS
    bool threw = false;
    try
    {
        (void)sirkit::interval_lis_tails(std::array{Interval{4, 1}});
    }
    catch (const std::invalid_argument &)
    {
        threw = true;
    }
    assert(threw);
#endif
}
