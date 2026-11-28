// SPDX-License-Identifier: CC0-1.0
#include "Geometry/Trapezoid_Count.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
using sirkit::TrapezoidCounts;
using sirkit::TrapezoidPoint;

auto edges(const auto &points, std::size_t a, std::size_t b)
{
    return std::pair(static_cast<std::int64_t>(points[b].first) - points[a].first,
                     static_cast<std::int64_t>(points[b].second) - points[a].second);
}

std::int64_t cross(auto a, auto b)
{
    return a.first * b.second - a.second * b.first;
}

TrapezoidCounts brute(std::span<const TrapezoidPoint> p)
{
    TrapezoidCounts count{};
    for (std::size_t i = 0; i < p.size(); ++i)
    {
        for (std::size_t j = i + 1; j < p.size(); ++j)
        {
            for (std::size_t k = j + 1; k < p.size(); ++k)
            {
                for (std::size_t l = k + 1; l < p.size(); ++l)
                {
                    std::array order{i, j, k, l};
                    do
                    {
                        const auto a = edges(p, order[0], order[1]), b = edges(p, order[1], order[2]);
                        const auto c = edges(p, order[2], order[3]), d = edges(p, order[3], order[0]);
                        if (cross(a, b) <= 0 || cross(b, c) <= 0 || cross(c, d) <= 0 || cross(d, a) <= 0)
                        {
                            continue;
                        }
                        const bool first = cross(a, c) == 0, second = cross(b, d) == 0;
                        count.trapezoids += first || second;
                        count.parallelograms += first && second;
                        break;
                    } while (std::next_permutation(order.begin() + 1, order.end()));
                }
            }
        }
    }
    return count;
}
}

int main()
{
    using sirkit::count_trapezoids;
    assert(count_trapezoids({}).trapezoids == 0);
    const std::array<TrapezoidPoint, 5> on_line{{{0, -3}, {0, -1}, {0, 0}, {0, 1}, {0, 3}}};
    const auto degenerate = count_trapezoids(on_line);
    assert(degenerate.trapezoids == 0 && degenerate.parallelograms == 0);
    constexpr auto high = std::numeric_limits<std::int32_t>::max();
    constexpr auto low = std::numeric_limits<std::int32_t>::min();
    const std::array<TrapezoidPoint, 4> box{{{low, low}, {high, low}, {high, high}, {low, high}}};
    const auto extreme = count_trapezoids(box);
    assert(extreme.trapezoids == 1 && extreme.parallelograms == 1);

    std::mt19937 random(418);
    for (int test = 0; test < 1600; ++test)
    {
        const std::size_t n = random() % 11;
        std::set<TrapezoidPoint> set;
        while (set.size() < n)
        {
            set.emplace(static_cast<int>(random() % 9) - 4, static_cast<int>(random() % 9) - 4);
        }
        std::vector<TrapezoidPoint> points(set.begin(), set.end());
        std::shuffle(points.begin(), points.end(), random);
        const auto actual = count_trapezoids(points), expected = brute(points);
        assert(actual.trapezoids == expected.trapezoids && actual.parallelograms == expected.parallelograms);
    }
    try
    {
        (void)count_trapezoids(std::array<TrapezoidPoint, 2>{{{0, 0}, {0, 0}}});
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    try
    {
        (void)count_trapezoids(std::vector<TrapezoidPoint>(92683));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
}
