// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/Furthest_Pair.cpp"
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
using Point = std::pair<std::int32_t, std::int32_t>;

std::uint64_t distance_squared(Point a, Point b)
{
    const auto dx = static_cast<std::int64_t>(a.first) - b.first;
    const auto dy = static_cast<std::int64_t>(a.second) - b.second;
    return static_cast<std::uint64_t>(dx * dx + dy * dy);
}

void compare(const std::vector<Point> &points)
{
    const auto answer = sirkit::furthest_pair(points);
    if (points.size() < 2)
    {
        assert(!answer);
        return;
    }

    std::uint64_t best = 0;
    for (std::size_t i = 0; i < points.size(); ++i)
    {
        for (std::size_t j = i + 1; j < points.size(); ++j)
        {
            best = std::max(best, distance_squared(points[i], points[j]));
        }
    }
    assert(answer);
    assert(answer->first_index < answer->second_index && answer->second_index < points.size());
    assert(answer->distance_squared == best);
    assert(distance_squared(points[answer->first_index], points[answer->second_index]) == best);
}
} // namespace

int main()
{
    compare({});
    compare({{0, 0}});
    compare({{0, 0}, {0, 0}});
    compare({{1, 0}, {1, 0}, {1, 0}, {1, 0}});
    compare({{3, 3}, {1, 1}, {2, 2}, {1, 1}, {0, 0}});
    compare({{0, 0}, {0, 9}, {12, 9}, {12, 0}, {0, 0}, {5, 6}});

    // 3×3の格子の全部分集合を照合し、平行な辺を持つ凸包も調べる。
    for (unsigned mask = 0; mask < (1U << 9); ++mask)
    {
        std::vector<Point> points;
        for (int i = 0; i < 9; ++i)
        {
            if (mask & (1U << i))
            {
                points.emplace_back(i / 3 - 1, i % 3 - 1);
            }
        }
        compare(points);
        std::ranges::reverse(points);
        compare(points);
    }

    std::mt19937 random(3924);
    for (int trial = 0; trial < 4000; ++trial)
    {
        std::vector<Point> points(random() % 50);
        for (auto &[x, y] : points)
        {
            x = static_cast<std::int32_t>(random() % 49) - 24;
            y = static_cast<std::int32_t>(random() % 49) - 24;
        }
        compare(points);
    }

    constexpr auto kLow = std::numeric_limits<std::int32_t>::min();
    constexpr auto kHigh = std::numeric_limits<std::int32_t>::max();
    constexpr std::uint64_t kWidth = std::numeric_limits<std::uint32_t>::max();
    const std::array<Point, 3> wide{{{kLow, 0}, {kHigh, 0}, {0, kHigh}}};
    const auto wide_answer = sirkit::furthest_pair(wide);
    assert(wide_answer->first_index == 0 && wide_answer->second_index == 1);
    assert(wide_answer->distance_squared == kWidth * kWidth);
    try
    {
        const std::array<Point, 4> too_far{{{kLow, kLow}, {kHigh, kLow}, {kHigh, kHigh}, {kLow, kHigh}}};
        sirkit::furthest_pair(too_far);
        assert(false);
    }
    catch (const std::overflow_error &)
    {
    }
}
