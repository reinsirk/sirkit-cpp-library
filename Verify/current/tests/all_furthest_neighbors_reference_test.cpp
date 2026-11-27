// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_ALL_FURTHEST_NEIGHBORS_DEFINED
#include "../../../Algorithm/Monge_Optimization.cpp"
#include "../../../Geometry/All_Furthest_Neighbors.cpp"
#endif
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Point = std::pair<std::int64_t, std::int64_t>;
using Polygon = std::vector<Point>;

std::int64_t square_distance(Point p, Point q)
{
    const auto dx = p.first - q.first, dy = p.second - q.second;
    return dx * dx + dy * dy;
}

std::int64_t cross(Point a, Point b, Point c)
{
    return (b.first - a.first) * (c.second - a.second) - (c.first - a.first) * (b.second - a.second);
}

Polygon gift_wrap(Polygon points)
{
    if (points.empty())
    {
        return {};
    }
    const auto start = std::ranges::min(points);
    auto at = start;
    Polygon polygon;
    do
    {
        polygon.push_back(at);
        auto next = at;
        for (const auto &candidate : points)
        {
            const auto turn = cross(at, next, candidate);
            if (turn < 0 || (turn == 0 && square_distance(at, candidate) > square_distance(at, next)))
            {
                next = candidate;
            }
        }
        at = next;
    } while (at != start);
    return polygon;
}

void verify_vertex(const Polygon &polygon, std::size_t row, int selected)
{
    assert(selected >= 0 && static_cast<std::size_t>(selected) < polygon.size());
    std::int64_t farthest = 0;
    for (const auto &p : polygon)
    {
        farthest = std::max(farthest, square_distance(polygon[row], p));
    }
    assert(square_distance(polygon[row], polygon[selected]) == farthest);
}

void compare(const Polygon &polygon)
{
    const auto result = sirkit::all_furthest_neighbors(polygon);
    assert(result.size() == polygon.size());
    for (std::size_t i = 0; i < result.size(); ++i)
    {
        verify_vertex(polygon, i, result[i]);
    }
}

template <class Exception, class Call>
void expect_exception(Call call)
{
    bool thrown = false;
    try
    {
        call();
    }
    catch (const Exception &)
    {
        thrown = true;
    }
    assert(thrown);
}

Polygon large_lattice_polygon()
{
    constexpr std::size_t kHalf = 250'000;
    Polygon directions{{1, 0}};
    for (int y = 1; directions.size() < kHalf; ++y)
    {
        for (int x = -512; x <= 512 && directions.size() < kHalf; ++x)
        {
            if (std::gcd(x, y) == 1)
            {
                directions.emplace_back(x, y);
            }
        }
    }
    std::ranges::sort(directions, [](Point a, Point b)
                      {
                          return cross({}, a, b) > 0;
                      });
    Polygon result;
    result.reserve(2 * directions.size());
    Point current{};
    for (int sign : {1, -1})
    {
        for (const auto &[dx, dy] : directions)
        {
            result.push_back(current);
            current.first += sign * dx;
            current.second += sign * dy;
        }
    }
    assert(current == Point{});
    return result;
}
} // namespace

int main()
{
    compare({});
    compare({{0, 0}});
    compare({{0, 0}, {0, 0}});
    compare({{0, 0}, {1, 3}, {-1, 3}});
    compare({{0, 0}, {4, 0}, {4, 4}, {0, 4}});
    // 頂点1の距離二乗は 484, 666, 648, 740, 576, 442。最初の極大値は最大値ではない。
    const Polygon non_unimodal{{-12, 10}, {-11, -11}, {11, -11}, {10, 4}, {7, 7}, {-3, 15}, {-11, 13}};
    compare(non_unimodal);
    assert(sirkit::all_furthest_neighbors(non_unimodal)[1] == 5);

    constexpr auto lo = std::numeric_limits<std::int64_t>::min();
    constexpr auto hi = std::numeric_limits<std::int64_t>::max();
    constexpr std::int64_t width = 2'000'000'000;
    compare({{lo, hi}, {lo + width, hi}, {lo + width, hi - width}, {lo, hi - width}});
    expect_exception<std::length_error>([]
                                        {
                                            (void)sirkit::detail::FurthestNeighborHeight(std::numeric_limits<std::size_t>::max());
                                        });
    expect_exception<std::overflow_error>([&]
                                          {
                                              (void)sirkit::all_furthest_neighbors(Polygon{{lo, 0}, {hi, 0}});
                                          });
    expect_exception<std::overflow_error>([&]
                                          {
                                              (void)sirkit::all_furthest_neighbors(Polygon{{0, 0}, {0, width + 1}});
                                          });
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::invalid_argument>([]
                                            {
                                                (void)sirkit::all_furthest_neighbors(Polygon{{0, 0}, {1, 0}, {2, 0}});
                                            });
    expect_exception<std::invalid_argument>([]
                                            {
                                                (void)sirkit::all_furthest_neighbors(Polygon{{0, 0}, {5, 0}, {0, 5}, {5, 5}});
                                            });
    expect_exception<std::invalid_argument>([]
                                            {
                                                (void)sirkit::all_furthest_neighbors(Polygon{{0, 0}, {4, 0}, {0, 4}, {0, 0}, {4, 0}, {0, 4}});
                                            });
#endif

    for (unsigned mask = 0; mask < (1U << 9); ++mask)
    {
        Polygon points;
        for (unsigned i = 0; i < 9; ++i)
        {
            if (mask & (1U << i))
            {
                points.emplace_back(i / 3, i % 3);
            }
        }
        auto hull = gift_wrap(points);
        compare(hull);
        std::ranges::reverse(hull);
        compare(hull);
    }

    std::mt19937 random(438195);
    for (int trial = 0; trial < 5500; ++trial)
    {
        Polygon points(random() % 40);
        const unsigned scale = trial % 2 == 0 ? 13 : 2'000'000'001;
        for (auto &[x, y] : points)
        {
            x = random() % scale;
            y = random() % scale;
        }
        auto hull = gift_wrap(points);
        if (!hull.empty())
        {
            std::ranges::rotate(hull, hull.begin() + random() % hull.size());
            if (trial % 3 == 0)
            {
                std::ranges::reverse(hull);
            }
        }
        compare(hull);
    }

    const auto maximum = large_lattice_polygon();
    assert(maximum.size() == 500'000);
    const auto result = sirkit::all_furthest_neighbors(maximum);
    for (unsigned i = 0; i < 48; ++i)
    {
        const auto row = random() % maximum.size();
        verify_vertex(maximum, row, result[row]);
    }
}
