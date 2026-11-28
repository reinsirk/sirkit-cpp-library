// SPDX-License-Identifier: CC0-1.0
#include "Geometry/Circle_Polygon_Intersection.cpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <numbers>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Point = std::pair<double, double>;
using sirkit::circle_polygon_intersection_area;

void Expect(bool value)
{
    if (!value)
    {
        std::abort();
    }
}

template <class Error, class F>
void ExpectThrow(F method)
{
    try
    {
        method();
    }
    catch (const Error &)
    {
        return;
    }
    std::abort();
}

void Near(double computed, long double expected)
{
    Expect(std::abs(computed - expected) <= 2e-9L * std::max(1.L, std::abs(expected)));
}

// Integrate vertical sections; rectangle corners only supply transition coordinates.
long double RectangleArea(long double x1, long double y1, long double x2, long double y2, long double r)
{
    x1 = std::max(x1, -r);
    x2 = std::min(x2, r);
    if (x1 >= x2)
    {
        return 0;
    }
    std::vector<long double> cuts{x1, x2};
    for (long double y : {y1, y2})
    {
        if (std::abs(y) <= r)
        {
            const auto t = std::sqrt(r * r - y * y);
            for (const auto x : {-t, t})
            {
                if (x1 < x && x < x2)
                {
                    cuts.push_back(x);
                }
            }
        }
    }
    std::sort(cuts.begin(), cuts.end());
    const auto primitive = [&](long double x)
    {
        const auto v = std::sqrt(std::max(0.L, r * r - x * x));
        return (x * v + r * r * std::asin(std::clamp(x / r, -1.L, 1.L))) / 2;
    };
    long double answer = 0;
    for (std::size_t i = 1; i < cuts.size(); ++i)
    {
        const auto start = cuts[i - 1];
        const auto end = cuts[i];
        const auto x = (start + end) / 2;
        const auto h = std::sqrt(r * r - x * x);
        if (y1 >= h || y2 <= -h)
        {
            continue;
        }
        const auto cap = primitive(end) - primitive(start);
        answer += (y2 <= h ? (end - start) * y2 : cap) -
                  (y1 >= -h ? (end - start) * y1 : -cap);
    }
    return answer;
}

void Random()
{
    std::mt19937 rng(47090213);
    for (int run = 0; run < 2200; ++run)
    {
        const int x0 = static_cast<int>(rng() % 400) - 200;
        const int y0 = static_cast<int>(rng() % 400) - 200;
        const int x1 = x0 + 1 + rng() % 100;
        const int y1 = y0 + 1 + rng() % 100;
        const int x2 = x1 + 1 + rng() % 100;
        const int y2 = y1 + 1 + rng() % 100;
        const int center_x = static_cast<int>(rng() % 2000) - 1000;
        const int center_y = static_cast<int>(rng() % 2000) - 1000;
        const double radius = 1 + rng() % 185;
        std::vector<Point> polygon{{center_x + x0, center_y + y0}, {center_x + x2, center_y + y0}, {center_x + x2, center_y + y1}, {center_x + x1, center_y + y1}, {center_x + x1, center_y + y2}, {center_x + x0, center_y + y2}};
        if (run % 3 == 1)
        {
            std::reverse(polygon.begin(), polygon.end());
        }
        if (run % 4 == 2)
        {
            polygon.push_back(polygon.front());
        }
        const long double expected = RectangleArea(x0, y0, x2, y1, radius) + RectangleArea(x0, y1, x1, y2, radius);
        Near(circle_polygon_intersection_area(polygon, radius, {center_x, center_y}), expected);
    }
}

void Boundaries()
{
    const std::vector<Point> large{{-20, -20}, {20, -20}, {20, 20}, {-20, 20}};
    Near(circle_polygon_intersection_area(large, 4), 16 * std::numbers::pi_v<long double>);
    const std::vector<Point> disjoint{{30, -20}, {40, -20}, {40, 20}, {30, 20}};
    Expect(circle_polygon_intersection_area(disjoint, 4) == 0);
    const std::vector<Point> tangent{{4, -20}, {40, -20}, {40, 20}, {4, 20}};
    Expect(circle_polygon_intersection_area(tangent, 4) == 0);
    const std::vector<Point> half{{0, -20}, {20, -20}, {20, 20}, {0, 20}};
    Near(circle_polygon_intersection_area(half, 4), 8 * std::numbers::pi_v<long double>);
    const std::vector<Point> tiny{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
    Expect(circle_polygon_intersection_area(tiny, 4) == 4);
    Expect(circle_polygon_intersection_area(tiny, 0) == 0);
    Expect(circle_polygon_intersection_area(tiny, std::numeric_limits<double>::denorm_min()) == 0);
    Expect(circle_polygon_intersection_area({}, 4) == 0);
    constexpr auto infinity = std::numeric_limits<double>::infinity();
    ExpectThrow<std::invalid_argument>([&]
                                       {
                                           (void)circle_polygon_intersection_area(large, -1);
                                       });
    ExpectThrow<std::invalid_argument>([&]
                                       {
                                           (void)circle_polygon_intersection_area(large, infinity);
                                       });
    ExpectThrow<std::invalid_argument>([&]
                                       {
                                           (void)circle_polygon_intersection_area(large, 1, {infinity, 0});
                                       });
    ExpectThrow<std::invalid_argument>([&]
                                       {
                                           (void)circle_polygon_intersection_area(large, 1, {0, infinity});
                                       });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)circle_polygon_intersection_area(large, 4, {1e142, 0});
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)circle_polygon_intersection_area(large, 4, {0, -1e142});
                                   });
}
}

int main()
{
    Random();
    Boundaries();
}
