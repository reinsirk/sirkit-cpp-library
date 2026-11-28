// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_DELAUNAY_TRIANGULATION_DEFINED
#include "../../../Geometry/Delaunay_Triangulation.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Point = sirkit::DelaunayPoint;
using Points = std::vector<Point>;

std::int64_t Orient(Point a, Point b, Point c)
{
    return (b.first - a.first) * (c.second - a.second) -
           (b.second - a.second) * (c.first - a.first);
}

std::int64_t Lift(Point a, Point query)
{
    const auto x = a.first - query.first;
    const auto y = a.second - query.second;
    return x * x + y * y;
}

bool Opposite(std::int64_t a, std::int64_t b)
{
    return (a < 0 && b > 0) || (a > 0 && b < 0);
}

void Check(const Points &points)
{
    const auto edges = sirkit::delaunay_triangulation(points);
    std::vector<std::size_t> unique;
    for (std::size_t i = 0; i < points.size(); ++i)
    {
        if (std::find(points.begin(), points.begin() + static_cast<std::ptrdiff_t>(i), points[i]) ==
            points.begin() + static_cast<std::ptrdiff_t>(i))
        {
            unique.push_back(i);
        }
    }
    if (unique.size() < 2)
    {
        assert(edges.empty());
        return;
    }
    if (std::ranges::all_of(unique, [&](std::size_t i)
                            {
                                return Orient(points[unique.front()], points[unique.back()], points[i]) == 0;
                            }))
    {
        std::ranges::sort(unique, {}, [&](std::size_t i)
                          {
                              return points[i];
                          });
        std::vector<sirkit::DelaunayEdge> expected;
        for (std::size_t i = 1; i < unique.size(); ++i)
        {
            expected.emplace_back(std::minmax(unique[i - 1], unique[i]));
        }
        auto actual = edges;
        std::ranges::sort(actual);
        std::ranges::sort(expected);
        assert(actual == expected);
        return;
    }

    std::vector<std::vector<bool>> linked(points.size(), std::vector<bool>(points.size()));
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto [a, b] = edges[i];
        assert(a < b && std::ranges::find(unique, a) != unique.end() && std::ranges::find(unique, b) != unique.end());
        assert(!linked[a][b]);
        linked[a][b] = true;
        for (const auto p : unique)
        {
            if (p != a && p != b && Orient(points[a], points[b], points[p]) == 0)
            {
                assert(points[p] < std::min(points[a], points[b]) || points[p] > std::max(points[a], points[b]));
            }
        }
        for (std::size_t j = 0; j < i; ++j)
        {
            const auto [c, d] = edges[j];
            assert(!(Opposite(Orient(points[a], points[b], points[c]), Orient(points[a], points[b], points[d])) &&
                     Opposite(Orient(points[c], points[d], points[a]), Orient(points[c], points[d], points[b]))));
        }
    }

    std::size_t boundary = 0, triangles = 0;
    for (const auto a : unique)
    {
        bool on_boundary = false;
        for (const auto b : unique)
        {
            if (a == b)
            {
                continue;
            }
            bool positive = false, negative = false;
            for (const auto c : unique)
            {
                const auto side = Orient(points[a], points[b], points[c]);
                positive = positive || side > 0;
                negative = negative || side < 0;
            }
            on_boundary = on_boundary || !(positive && negative);
            if (a >= b || !linked[a][b])
            {
                continue;
            }
            for (const auto c : unique)
            {
                if (b >= c || !linked[a][c] || !linked[b][c])
                {
                    continue;
                }
                const auto direction = Orient(points[a], points[b], points[c]) > 0 ? 1 : -1;
                if (std::ranges::any_of(unique, [&](std::size_t p)
                                        {
                                            return direction * Orient(points[a], points[b], points[p]) > 0 &&
                                                   direction * Orient(points[b], points[c], points[p]) > 0 &&
                                                   direction * Orient(points[c], points[a], points[p]) > 0;
                                        }))
                {
                    continue;
                }
                ++triangles;
                for (const auto p : unique)
                {
                    const auto circle = Lift(points[a], points[p]) * Orient(points[p], points[b], points[c]) +
                                        Lift(points[b], points[p]) * Orient(points[p], points[c], points[a]) +
                                        Lift(points[c], points[p]) * Orient(points[p], points[a], points[b]);
                    assert(direction * circle <= 0);
                }
            }
        }
        boundary += on_boundary;
    }
    assert(edges.size() == 3 * unique.size() - 3 - boundary);
    assert(triangles == 2 * unique.size() - 2 - boundary);
}
} // namespace

int main()
{
    Check({});
    Check({{1, 1}});
    Check({{2, 2}, {2, 2}});
    Check({{2, 2}, {0, 0}, {1, 1}, {2, 2}, {0, 0}});
    const Points vertices{{0, 0}, {4, 0}, {1, 3}, {1, 1}};
    const auto triangulation = sirkit::delaunay_triangulation(vertices);
    for (const auto &edge : sirkit::euclidean_mst(vertices))
    {
        assert(std::ranges::find(triangulation, sirkit::DelaunayEdge(std::minmax(edge.from, edge.to))) != triangulation.end());
    }
    constexpr auto lo = std::numeric_limits<std::int64_t>::min();
    constexpr auto hi = std::numeric_limits<std::int64_t>::max();
    Check({{lo, hi}, {lo + 25'000, hi}, {lo, hi - 25'000}, {lo + 25'000, hi - 25'000}, {lo + 180, hi - 328}, {lo + 25'000, hi}, {lo, hi - 3}});

    constexpr std::int64_t kWidth = 2'000'000'000;
    for (const auto &out_of_range : {Points{{lo, 0}, {hi, 0}}, Points{{0, 0}, {0, kWidth + 1}}})
    {
        bool threw = false;
        try
        {
            (void)sirkit::delaunay_triangulation(out_of_range);
        }
        catch (const std::overflow_error &)
        {
            threw = true;
        }
        assert(threw);
    }
    const auto close_circle = sirkit::delaunay_triangulation(Points{{0, 0}, {kWidth, 0}, {kWidth, kWidth}, {0, kWidth - 1}});
    assert(std::ranges::find(close_circle, sirkit::DelaunayEdge{1, 3}) != close_circle.end());
    assert(std::ranges::find(close_circle, sirkit::DelaunayEdge{0, 2}) == close_circle.end());
    for (unsigned mask = 0; mask < (1U << 9); ++mask)
    {
        Points points;
        for (int i = 0; i < 9; ++i)
        {
            if (mask & (1U << i))
            {
                points.emplace_back(i / 3, i % 3);
            }
        }
        Check(points);
    }
    std::mt19937 random(315913);
    for (int trial = 0; trial < 1200; ++trial)
    {
        Points points(random() % 26);
        for (auto &[x, y] : points)
        {
            const unsigned span = trial % 2 == 0 ? 9U : 25'001U;
            x = random() % span;
            y = random() % span;
        }
        Check(points);
        if (trial % 3 == 0)
        {
            auto scaled = points;
            for (auto &[x, y] : scaled)
            {
                x = lo + x * 80'000;
                y = hi - kWidth + y * 80'000;
            }
            assert(sirkit::delaunay_triangulation(scaled) == sirkit::delaunay_triangulation(points));
        }
    }
}
