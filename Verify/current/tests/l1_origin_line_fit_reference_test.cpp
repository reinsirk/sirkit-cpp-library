// SPDX-License-Identifier: CC0-1.0
#include "Geometry/L1_Origin_Line_Fit.cpp"

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
using sirkit::Fraction;

Fraction cost_at(const std::vector<Point> &points, Fraction slope, bool swapped)
{
    Fraction total;
    for (const auto &[x, y] : points)
    {
        total += (Fraction(swapped ? y : x) * slope - Fraction(swapped ? x : y)).abs();
    }
    return total;
}

Fraction brute(const std::vector<Point> &points)
{
    auto best = cost_at(points, Fraction(0), false);
    for (const bool swapped : {false, true})
    {
        for (const auto slope : {Fraction(-1), Fraction(0), Fraction(1)})
        {
            best = std::min(best, cost_at(points, slope, swapped));
        }
        for (const auto &[x, y] : points)
        {
            const auto a = swapped ? y : x, b = swapped ? x : y;
            if (a)
            {
                const Fraction slope(b, a);
                if (slope.abs() <= 1)
                {
                    best = std::min(best, cost_at(points, slope, swapped));
                }
            }
        }
    }
    return best;
}
}

int main()
{
    using sirkit::l1_origin_line_fit;
    const auto empty = l1_origin_line_fit({});
    assert(empty.cost == 0 && empty.slope == 0 && !empty.swapped);
    const auto vertical = l1_origin_line_fit(std::array<Point, 2>{{{0, 3}, {0, -4}}});
    assert(vertical.cost == 0 && vertical.slope == 0 && vertical.swapped);
    const auto horizontal = l1_origin_line_fit(std::array<Point, 3>{{{4, 3}, {1, 1}, {-8, -6}}});
    assert(horizontal.cost == Fraction(1, 4) && horizontal.slope == Fraction(3, 4) && !horizontal.swapped);

    std::mt19937 rng(5164);
    for (int trial = 0; trial < 1800; ++trial)
    {
        std::vector<Point> points(rng() % 14);
        for (auto &[x, y] : points)
        {
            x = static_cast<int>(rng() % 19) - 9;
            y = static_cast<int>(rng() % 19) - 9;
        }
        const auto result = l1_origin_line_fit(points);
        assert(result.slope.abs() <= 1);
        assert(cost_at(points, result.slope, result.swapped) == result.cost);
        assert(result.cost == brute(points));
    }

    constexpr auto kLo = std::numeric_limits<std::int32_t>::min();
    constexpr auto kHi = std::numeric_limits<std::int32_t>::max();
    const auto extreme = l1_origin_line_fit(std::array<Point, 2>{{{kLo, kLo}, {kLo, kHi}}});
    assert(extreme.cost == INT64_C(4294967295));
    std::vector<Point> overflow(6, {kHi, 1});
    overflow.insert(overflow.end(), 3, {1, kHi});
    overflow.emplace_back(0, kHi);
    try
    {
        (void)l1_origin_line_fit(overflow);
        assert(false);
    }
    catch (const std::overflow_error &)
    {
    }
    try
    {
        sirkit::detail::CheckOriginFitSize(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
}
