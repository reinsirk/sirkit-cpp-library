// SPDX-License-Identifier: CC0-1.0
#include "Geometry/Polygon_Chord_Moments.cpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Moments = sirkit::PolygonChordMoments;
using Point = Moments::Point;

void Expect(bool condition)
{
    if (!condition)
    {
        std::abort();
    }
}

template <class Exception, class F>
void ExpectThrow(F operation)
{
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        return;
    }
    std::abort();
}

std::int64_t Cross(const Point &a, const Point &b, const Point &c)
{
    const auto x1 = b.first - a.first;
    const auto y1 = b.second - a.second;
    const auto x2 = c.first - a.first;
    const auto y2 = c.second - a.second;
    return x1 * y2 - y1 * x2;
}

std::vector<Point> MakeHull(std::vector<Point> points)
{
    std::sort(points.begin(), points.end());
    std::vector<Point> hull;
    const auto build = [&](auto first, auto last)
    {
        const auto offset = hull.size();
        for (auto i = first; i != last; ++i)
        {
            while (hull.size() > offset + 1 && Cross(hull[hull.size() - 2], hull.back(), *i) <= 0)
            {
                hull.pop_back();
            }
            hull.push_back(*i);
        }
        hull.pop_back();
    };
    build(points.begin(), points.end());
    build(points.rbegin(), points.rend());
    return hull;
}

void CheckResult(const Moments::Result &result, const std::vector<Point> &polygon)
{
    std::int64_t area2 = 0;
    long double x = 0, y = 0;
    for (std::size_t i = 2; i < polygon.size(); ++i)
    {
        const auto &a = polygon[0];
        const auto &b = polygon[i - 1];
        const auto &c = polygon[i];
        const auto cross = Cross(a, b, c);
        area2 += cross;
        x += static_cast<long double>(cross) * (a.first + b.first + c.first);
        y += static_cast<long double>(cross) * (a.second + b.second + c.second);
    }
    Expect(result.signed_area2 == area2);
    Expect(result.centroid.has_value() == (area2 != 0));
    if (area2 != 0)
    {
        const auto [actual_x, actual_y] = *result.centroid;
        Expect(std::abs(x / (3.L * area2) - actual_x) <= 3e-7L);
        Expect(std::abs(y / (3.L * area2) - actual_y) <= 3e-7L);
    }
}

void Differential()
{
    std::mt19937 rng(2973813);
    for (int trial = 0; trial < 310; ++trial)
    {
        const std::int64_t magnitude = trial % 3 == 0 ? 999999999 : 7000;
        const auto coordinate = [&]()
        {
            return static_cast<std::int64_t>(rng() % (2 * magnitude + 1)) - magnitude;
        };
        std::vector<Point> points;
        for (int i = 0; i < 30; ++i)
        {
            const auto x = coordinate();
            const auto y = coordinate();
            points.emplace_back(x, y);
        }
        auto hull = MakeHull(std::move(points));
        if (trial % 2 == 1)
        {
            std::reverse(hull.begin(), hull.end());
        }
        const Moments moments(hull);
        CheckResult(moments.total(), hull);
        for (std::size_t a = 0; a < hull.size(); ++a)
        {
            for (std::size_t b = 0; b < hull.size(); ++b)
            {
                std::vector<Point> slice{hull[a]};
                for (std::size_t i = a; i != b;)
                {
                    i = (i + 1) % hull.size();
                    slice.push_back(hull[i]);
                }
                CheckResult(moments.slice(a, b), slice);
            }
        }
    }
}

void LimitsAndLifetime()
{
    using Number = std::int64_t;
    constexpr Number k = 1000000000;
    const std::vector<Point> box{{-k, -k}, {k, -k}, {k, k}, {-k, k}};
    const Moments polygon(box);
    const auto area = polygon.total();
    Expect(polygon.size() == 4);
    Expect(area.signed_area2 == 8 * k * k && area.centroid == std::pair{0., 0.});
    auto copy = polygon;
    const Moments moved(std::move(copy));
    CheckResult(moved.slice(0, 2), std::vector<Point>{box.begin(), box.begin() + 3});
    Expect(copy.size() == 0 && !copy.total().centroid && copy.total().signed_area2 == 0);
    copy = moved;
    Expect(copy.size() == 4);
    const Moments empty({});
    Expect(empty.size() == 0 && !empty.total().centroid);
    const std::vector<Point> segment{{10, 8}, {3, 5}};
    Expect(!Moments(segment).total().centroid);
    const std::vector<Point> slender{
        {k - 10, k - 9},
        {k - 8, k - 7},
        {k - 9, k - 7},
    };
    Expect(Moments(slender).total().signed_area2 == 2);
    Expect(Moments(slender).total().centroid->first == k - 9.);

#ifndef SKIP_RUNTIME_CHECKS
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)moved.slice(4, 1);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)moved.slice(2, 4);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)empty.slice(0, 0);
                                   });
#endif
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       const std::vector<Point> vertices{{0, k + 1}};
                                       Moments outside(vertices);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       const std::vector<Point> vertices{{-k - 1, 0}};
                                       Moments outside(vertices);
                                   });

    // Twice the same oriented boundary has an integral area outside int64.
    auto twice = box;
    twice.insert(twice.end(), box.begin(), box.end());
    ExpectThrow<std::overflow_error>([&]
                                     {
                                         (void)Moments(twice).total();
                                     });
}
}

int main()
{
    Differential();
    LimitsAndLifetime();
}
