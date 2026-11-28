// SPDX-License-Identifier: CC0-1.0
#include "Geometry/Half_Plane_Intersection.cpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using IntegerPoint = std::pair<std::int64_t, std::int64_t>;
using Point = std::pair<double, double>;
using sirkit::HalfPlane;

void Expect(bool condition)
{
    if (!condition)
    {
        std::abort();
    }
}

template <class Error, class F>
void ExpectThrow(F operation)
{
    try
    {
        operation();
    }
    catch (const Error &)
    {
        return;
    }
    std::abort();
}

bool Near(const Point &a, const Point &b)
{
    return std::abs(a.first - b.first) <= 1e-9 && std::abs(a.second - b.second) <= 1e-9;
}

std::vector<Point> Brute(const std::vector<HalfPlane> &constraints, const IntegerPoint &lo, const IntegerPoint &hi)
{
    auto lines = constraints;
    lines.insert(lines.end(), {{1, 0, hi.first}, {-1, 0, -lo.first}, {0, 1, hi.second}, {0, -1, -lo.second}});
    std::vector<Point> points;
    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        for (std::size_t j = 0; j < i; ++j)
        {
            const auto [a, b, c] = lines[i];
            const auto [d, e, f] = lines[j];
            auto det = a * e - b * d;
            auto x = c * e - b * f;
            auto y = a * f - c * d;
            if (det == 0)
            {
                continue;
            }
            if (det < 0)
            {
                det = -det;
                x = -x;
                y = -y;
            }
            const bool feasible = std::ranges::all_of(lines, [&](const auto &h)
                                                      {
                                                          return h.a * x + h.b * y <= h.c * det;
                                                      });
            if (feasible)
            {
                const Point p(static_cast<double>(x) / det, static_cast<double>(y) / det);
                if (std::ranges::none_of(points, [&](const auto &v)
                                         {
                                             return Near(p, v);
                                         }))
                {
                    points.push_back(p);
                }
            }
        }
    }
    return points;
}

void Compare(std::vector<HalfPlane> lines, IntegerPoint lo, IntegerPoint hi)
{
    const auto current = sirkit::half_plane_intersection(lines, lo, hi);
    const auto expected = Brute(lines, lo, hi);
    bool equal = current.size() == expected.size();
    for (const auto &p : expected)
    {
        equal &= std::ranges::count_if(current, [&](const auto &v)
                                       {
                                           return Near(p, v);
                                       }) == 1;
    }
    if (!equal)
    {
        std::cerr << "bounds " << lo.first << ' ' << lo.second << ' ' << hi.first << ' ' << hi.second << '\n';
        for (const auto &[a, b, c] : lines)
        {
            std::cerr << a << ' ' << b << ' ' << c << '\n';
        }
        std::cerr << "vertices " << current.size() << ' ' << expected.size() << '\n';
        std::abort();
    }
    for (std::size_t i = 0; current.size() > 2 && i < current.size(); ++i)
    {
        const auto [ax, ay] = current[i];
        const auto [bx, by] = current[(i + 1) % current.size()];
        const auto [cx, cy] = current[(i + 2) % current.size()];
        Expect((bx - ax) * (cy - by) - (by - ay) * (cx - bx) > -1e-9);
    }
}

void Random()
{
    std::mt19937 rng(5321838);
    for (int iter = 0; iter < 7500; ++iter)
    {
        const int n = rng() % 16;
        std::vector<HalfPlane> lines;
        for (int i = 0; i < n; ++i)
        {
            const int a = static_cast<int>(rng() % 11) - 5;
            const int b = static_cast<int>(rng() % 11) - 5;
            const int c = static_cast<int>(rng() % 71) - 30;
            lines.push_back({a, b, c});
            if (rng() % 5 == 0)
            {
                lines.push_back({2 * a, 2 * b, 2 * c});
            }
        }
        const int low_x = static_cast<int>(rng() % 11) - 10;
        const int low_y = static_cast<int>(rng() % 11) - 10;
        const int delta_x = iter % 4 == 0 ? 0 : rng() % 26;
        const int delta_y = iter % 9 == 0 ? 0 : rng() % 26;
        Compare(std::move(lines), {low_x, low_y}, {low_x + delta_x, low_y + delta_y});
    }
}

void Limits()
{
    Compare({}, {-8, -8}, {9, 10});
    Compare({{0, 0, 3}, {1, 1, 0}, {-1, -1, 0}}, {-5, -5}, {5, 5});
    Compare({{0, 0, -1}}, {0, 0}, {2, 2});
    Compare({{1, 0, 2}, {-1, 0, -2}, {0, 1, 3}, {0, -1, -3}}, {-10, -10}, {10, 10});
    Compare({{1, 0, 0}, {-1, 0, -10}}, {-20, -20}, {20, 20});
    constexpr std::int64_t k = 1000000000;
    const std::vector<HalfPlane> extreme{
        {2 * k, 2 * k, 2 * k * k},
        {-2 * k, 2 * k, 2 * k * k},
        {-2 * k, -2 * k, 2 * k * k},
        {2 * k, -2 * k, 2 * k * k},
    };
    const auto diamond = sirkit::half_plane_intersection(extreme, {-k, -k}, {k, k});
    Expect(diamond.size() == 4);
    for (auto [x, y] : diamond)
    {
        Expect(std::abs(x) + std::abs(y) == k);
    }
    constexpr std::int64_t x = 210000;
    constexpr std::int64_t y = -100000;
    const std::vector<HalfPlane> near_parallel{
        {2 * k, 2 * k - 1, 2 * k * x + (2 * k - 1) * y},
        {2 * k - 1, 2 * k - 2, (2 * k - 1) * x + (2 * k - 2) * y},
    };
    const auto polygon = sirkit::half_plane_intersection(near_parallel, {-k, -k}, {k, k});
    Expect(std::ranges::any_of(polygon, [&](const auto &p)
                               {
                                   return Near(p, {x, y});
                               }));
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)sirkit::half_plane_intersection({}, {-k - 1, 0}, {k, k});
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)sirkit::half_plane_intersection({}, {0, 0}, {k, k + 1});
                                   });
    ExpectThrow<std::invalid_argument>([]
                                       {
                                           (void)sirkit::half_plane_intersection({}, {1, 2}, {0, 4});
                                       });
    ExpectThrow<std::invalid_argument>([]
                                       {
                                           (void)sirkit::half_plane_intersection({}, {1, 2}, {5, 1});
                                       });
    for (const HalfPlane h : {HalfPlane{2 * k + 1, 0, 0}, {0, -2 * k - 1, 0}, {0, 0, 2 * k * k + 1}})
    {
        ExpectThrow<std::out_of_range>([&]
                                       {
                                           (void)sirkit::half_plane_intersection(std::span(&h, 1), {-k, -k}, {k, k});
                                       });
    }
    const std::vector<IntegerPoint> a{{0, 0}, {4, 0}, {4, 4}, {0, 4}};
    const std::vector<IntegerPoint> b{{4, 0}, {7, 0}, {7, 4}, {4, 4}};
    const std::vector<IntegerPoint> c{{4, 4}, {7, 4}, {7, 8}, {4, 8}};
    Expect(sirkit::convex_polygon_intersection(a, b).size() == 2);
    Expect(sirkit::convex_polygon_intersection(a, c).size() == 1);
    Expect(sirkit::convex_polygon_intersection({}, b).empty());
    Expect(sirkit::convex_polygon_intersection(a, {}).empty());
}
}

int main()
{
    Random();
    Limits();
}
