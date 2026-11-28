// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_INTERVAL_SCHEDULING_DEFINED
#include "../../../Algorithm/Interval_Scheduling.cpp"
#endif
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
void check(std::span<const sirkit::ScheduleInterval> intervals)
{
    std::size_t best_count = 0;
    std::int64_t best_weight = 0;
    const auto total = std::uint64_t{1} << intervals.size();
    for (std::uint64_t mask = 0; mask < total; ++mask)
    {
        std::size_t count = 0;
        std::int64_t weight = 0;
        bool feasible = true;
        for (std::size_t i = 0; i < intervals.size(); ++i)
        {
            if (((mask >> i) & 1U) == 0)
            {
                continue;
            }
            ++count;
            weight += intervals[i].weight;
            for (std::size_t j = 0; j < i; ++j)
            {
                if (((mask >> j) & 1U) && intervals[i].left < intervals[j].right &&
                    intervals[j].left < intervals[i].right)
                {
                    feasible = false;
                }
            }
        }
        if (feasible)
        {
            best_count = std::max(best_count, count);
            best_weight = std::max(best_weight, weight);
        }
    }

    const auto check_selection = [&](std::span<const std::size_t> selected) {
        auto last = std::numeric_limits<std::int64_t>::min();
        std::int64_t weight = 0;
        for (const auto id : selected)
        {
            assert(id < intervals.size());
            assert(last <= intervals[id].left);
            last = intervals[id].right;
            weight += intervals[id].weight;
        }
        return weight;
    };
    const auto selected = sirkit::interval_schedule_max_count(intervals);
    assert(selected.size() == best_count);
    check_selection(selected);
    const auto weighted = sirkit::interval_schedule_max_weight(intervals);
    assert(weighted.weight == best_weight);
    assert(check_selection(weighted.indices) == best_weight);
}

template<class Exception, class Function>
void check_error(Function run)
{
    bool thrown = false;
    try
    {
        run();
    }
    catch (const Exception &)
    {
        thrown = true;
    }
    assert(thrown);
}
} // namespace

int main()
{
    using Interval = sirkit::ScheduleInterval;
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    check({});
    check(std::array{Interval{-8, -3, -4}, Interval{-3, 1, -1}, Interval{1, 3, 0}});
    check(std::array{Interval{kMin, -1, kMax}, Interval{-1, 0, kMin}, Interval{0, kMax, 0}});
    check(std::array{Interval{0, 4, 10}, Interval{0, 2, 3}, Interval{2, 4, 2}, Interval{-1, 4, 1}});
    check(std::array{Interval{-1, 0}, Interval{0, 2}, Interval{2, 3}});
    std::mt19937 random(286003);
    for (int trial = 0; trial < 500; ++trial)
    {
        std::vector<Interval> intervals(random() % 12);
        for (auto &[left, right, weight] : intervals)
        {
            left = static_cast<std::int64_t>(random() % 21) - 10;
            right = left + 1 + static_cast<std::int64_t>(random() % 6);
            weight = static_cast<std::int64_t>(random() % 21) - 10;
        }
        check(intervals);
    }

    constexpr std::int64_t kLength = 200'000;
    std::vector<Interval> intervals;
    intervals.reserve(kLength);
    for (auto i = kLength; i > 0; --i)
    {
        intervals.push_back({i - 1, i, 1'000'000'000});
    }
    const auto result = sirkit::interval_schedule_max_weight(intervals);
    const auto count = sirkit::interval_schedule_max_count(intervals);
    assert(result.weight == kLength * 1'000'000'000);
    assert(result.indices == count && count.size() == kLength);
    for (std::size_t i = 0; i < count.size(); ++i)
    {
        assert(count[i] == count.size() - i - 1);
    }

    check_error<std::overflow_error>([&] {
        (void)sirkit::interval_schedule_max_weight(std::array{Interval{0, 1, kMax}, Interval{1, 2, 1}});
    });
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::invalid_argument>([] {
        (void)sirkit::interval_schedule_max_count(std::array{Interval{0, 0}});
    });
    check_error<std::invalid_argument>([] {
        (void)sirkit::interval_schedule_max_weight(std::array{Interval{2, 1}});
    });
#endif
}
