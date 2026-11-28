// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Interval_Cover_Subsets.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Choice = sirkit::IntervalCoverChoice<std::uint64_t>;

std::uint64_t enumerate(std::int64_t left, std::int64_t right, const std::vector<Choice> &choices)
{
    std::uint64_t total = 0;
    for (std::size_t mask = 0; mask < (std::size_t{1} << choices.size()); ++mask)
    {
        std::vector<std::pair<std::int64_t, std::int64_t>> selected;
        std::uint64_t score = 1;
        for (std::size_t i = 0; i < choices.size(); ++i)
        {
            const auto &interval = choices[i];
            if ((mask >> i) & 1)
            {
                selected.emplace_back(interval.left, interval.right);
                score *= interval.take;
            }
            else
            {
                score *= interval.omit;
            }
        }
        std::ranges::sort(selected);
        auto end = left;
        for (const auto &[begin, until] : selected)
        {
            if (begin > end)
            {
                break;
            }
            end = std::max(end, until);
        }
        if (end >= right)
        {
            total += score;
        }
    }
    return total;
}

void check(std::int64_t left, std::int64_t right, const std::vector<Choice> &choices)
{
    assert(sirkit::count_interval_cover_subsets<std::uint64_t>(left, right, choices) ==
           enumerate(left, right, choices));
}

template <class Action>
void expect_invalid(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught);
}
} // namespace

int main()
{
    check(0, 0, {});
    check(0, 1, {});
    check(1, 3, {{0, 1}, {2, 2}, {3, 10}, {-10, -9}});
    check(0, 4, {{0, 4, 0, 7}, {0, 1, 2, 0}, {1, 2, 3, 5}, {2, 6, 4, 8}, {-5, -4, 2, 9}});
    check(-1, 5, {{0, 2}, {-1, 1}, {2, 3}, {2, 5}, {4, 9}, {-8, 0}});
    check(0, 2, {{0, 1}, {0, 1}, {0, 2}, {1, 2}, {1, 2}});
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    check(kMin, kMax, {{kMin, 0}, {0, kMax}, {kMin, kMax}, {kMax, kMax}, {kMin, kMin}});
    check(kMax, kMax, {{kMin, kMax, 3, 4}, {0, 1, 0, 2}});

    expect_invalid([]
                   {
                       (void)sirkit::count_interval_cover_subsets<int>(1, 0, {});
                   });
    expect_invalid([]
                   {
                       const std::array reversed{Choice{3, -2}};
                       (void)sirkit::count_interval_cover_subsets<std::uint64_t>(0, 4, reversed);
                   });

    std::mt19937 random(4509371);
    for (int trial = 0; trial < 820; ++trial)
    {
        const auto left = static_cast<std::int64_t>(random() % 7) - 7;
        const auto right = left + static_cast<std::int64_t>(random() % 15);
        std::vector<Choice> choices(random() % 12);
        for (auto &interval : choices)
        {
            const auto a = static_cast<std::int64_t>(random() % 29) - 12;
            const auto b = static_cast<std::int64_t>(random() % 29) - 12;
            interval = {std::min(a, b), std::max(a, b), random() % 5, random() % 5};
        }
        check(left, right, choices);
    }
}
