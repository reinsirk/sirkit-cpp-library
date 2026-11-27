// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/Minimum_Enclosing_Circle.cpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Point = sirkit::MinimumEnclosingCircle::Point;

struct ReferenceCircle
{
    long double x, y, square;
};

long double square(long double x)
{
    return x * x;
}

long double distance(ReferenceCircle circle, Point p)
{
    return square(circle.x - p.first) + square(circle.y - p.second);
}

ReferenceCircle reference(const std::vector<Point> &points)
{
    ReferenceCircle best{0, 0, std::numeric_limits<long double>::infinity()};
    const auto visit = [&](long double x, long double y, Point boundary)
    {
        ReferenceCircle circle{x, y, 0};
        circle.square = distance(circle, boundary);
        const auto error = 1e-12L * (circle.square + 1);
        if (circle.square < best.square && std::ranges::all_of(points, [&](Point p)
                                                               {
                                                                   return distance(circle, p) <= circle.square + error;
                                                               }))
        {
            best = circle;
        }
    };
    for (std::size_t i = 0; i < points.size(); ++i)
    {
        const auto [x, y] = points[i];
        visit(x, y, points[i]);
        for (std::size_t j = 0; j < i; ++j)
        {
            const auto [bx, by] = points[j];
            visit((x + bx) / 2.0L, (y + by) / 2.0L, points[i]);
            for (std::size_t k = 0; k < j; ++k)
            {
                const auto [cx, cy] = points[k];
                // 三辺の二等分線を、絶対座標で交わらせる。
                const auto ux = 2.0L * (bx - x), uy = 2.0L * (by - y);
                const auto vx = 2.0L * (cx - x), vy = 2.0L * (cy - y);
                const auto first = square(bx) + square(by) - square(x) - square(y);
                const auto second = square(cx) + square(cy) - square(x) - square(y);
                const auto determinant = ux * vy - uy * vx;
                if (determinant != 0)
                {
                    visit((first * vy - second * uy) / determinant,
                          (ux * second - vx * first) / determinant, points[i]);
                }
            }
        }
    }
    return best;
}

void compare(const std::vector<Point> &points, std::uint64_t seed)
{
    const auto answer = sirkit::minimum_enclosing_circle(points, seed);
    if (points.empty())
    {
        assert(!answer);
        return;
    }
    assert(answer);
    const auto expected = reference(points);
    assert(std::isfinite(expected.square));
    const auto error = 1e-10L * (expected.square + 1);
    const auto [x, y] = answer->center();
    assert(std::abs(x - expected.x) <= 1e-10L * (std::abs(expected.x) + 1));
    assert(std::abs(y - expected.y) <= 1e-10L * (std::abs(expected.y) + 1));
    assert(std::abs(answer->radius_squared() - expected.square) <= error);
    for (const auto &p : points)
    {
        assert(answer->contains(p));
        assert(answer->on_boundary(p) == (std::abs(distance(expected, p) - expected.square) <= error));
    }
}
} // namespace

int main()
{
    compare({}, 0);
    compare({{0, 0}}, 0);
    compare({{4, -2}, {4, -2}, {4, -2}}, 5);
    compare({{-1, 0}, {0, 0}, {1, 0}}, 4);
    compare({{-10000, 0}, {10000, 0}, {0, 10000}, {0, -9999}, {0, -10000}}, 7);
    compare({{-10000, -10000}, {-9999, -10000}, {10000, 9999}, {9999, 9998}}, 2);
    compare({{3, -1}, {4, 2}, {0, 3}}, 0);

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
        compare(points, mask);
        compare(points, mask + 5);
    }

    std::mt19937 random(91476);
    for (int trial = 0; trial < 2500; ++trial)
    {
        std::vector<Point> points(random() % 13);
        const std::int32_t range = trial % 2 == 0 ? 10000 : 3;
        for (auto &[x, y] : points)
        {
            x = static_cast<std::int32_t>(random() % (2 * range + 1)) - range;
            y = static_cast<std::int32_t>(random() % (2 * range + 1)) - range;
        }
        compare(points, random());
    }

    const std::array<Point, 2> diameter{{{0, 0}, {2, 0}}};
    const auto circle = sirkit::minimum_enclosing_circle(diameter);
    assert(circle->contains({1, 0}) && !circle->on_boundary({1, 0}));
    assert(!circle->contains({3, 0}) && !circle->on_boundary({3, 0}));
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &point : {Point{-10001, 0}, Point{10001, 0}, Point{0, -10001}, Point{0, 10001}})
    {
        try
        {
            static_cast<void>(circle->contains(point));
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
    try
    {
        const std::array<Point, 1> points{{{0, 10001}}};
        sirkit::minimum_enclosing_circle(points);
        assert(false);
    }
    catch (const std::out_of_range &)
    {
    }
#endif
}
