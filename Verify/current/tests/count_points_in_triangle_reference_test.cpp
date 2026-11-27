// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_COUNT_POINTS_IN_TRIANGLE_DEFINED
#include "../../../Geometry/Count_Points_In_Triangle.cpp"
#endif
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Point = sirkit::CountPointsInTriangle::Point;
using Points = std::vector<Point>;

std::int64_t area(Point a, Point b, Point c)
{
    return (a.first - c.first) * (b.second - c.second) - (a.second - c.second) * (b.first - c.first);
}

std::size_t reference(Point a, Point b, Point c, const Points &points)
{
    const auto outer = area(a, b, c);
    std::size_t result = 0;
    for (const auto &p : points)
    {
        const auto x = area(p, b, c), y = area(a, p, c), z = area(a, b, p);
        if (outer > 0)
        {
            result += x > 0 && y > 0 && z > 0;
        }
        if (outer < 0)
        {
            result += x < 0 && y < 0 && z < 0;
        }
    }
    return result;
}

void compare(const Points &vertices, const Points &points)
{
    const sirkit::CountPointsInTriangle count(vertices, points);
    for (std::size_t a = 0; a < vertices.size(); ++a)
    {
        for (std::size_t b = 0; b < vertices.size(); ++b)
        {
            for (std::size_t c = 0; c < vertices.size(); ++c)
            {
                assert(count.count(a, b, c) == reference(vertices[a], vertices[b], vertices[c], points));
            }
        }
    }
}

void expect_overflow(const Points &vertices, const Points &points)
{
    bool threw = false;
    try
    {
        (void)sirkit::CountPointsInTriangle(vertices, points);
    }
    catch (const std::overflow_error &)
    {
        threw = true;
    }
    assert(threw);
}

#ifndef SKIP_RUNTIME_CHECKS
void invalid_index(const sirkit::CountPointsInTriangle &triangles, std::array<std::size_t, 3> query)
{
    bool threw = false;
    try
    {
        (void)triangles.count(query[0], query[1], query[2]);
    }
    catch (const std::out_of_range &)
    {
        threw = true;
    }
    assert(threw);
}
#endif
} // namespace

int main()
{
    compare({}, {});
    compare({}, {{0, 0}});
    compare({{0, 0}, {1, 0}, {1, 1}}, {});
    compare({{0, 0}, {0, 0}, {4, 0}, {0, 4}, {4, 4}, {4, 2}, {1, 3}},
            {{0, 0}, {0, 0}, {4, 4}, {1, 1}, {0, 1}, {1, 0}, {2, 2}, {2, 2}, {3, 3}, {1, 3}, {-1, 0}});
    compare({{0, 4}, {2, 0}, {4, 4}, {4, -3}, {0, -3}, {1, 2}},
            {{2, 0}, {2, 0}, {1, 2}, {3, 2}, {2, 1}, {2, 4}, {1, 3}, {0, 4}, {4, 4}});

    constexpr auto low = std::numeric_limits<std::int64_t>::min();
    constexpr auto high = std::numeric_limits<std::int64_t>::max();
    constexpr std::int64_t width = 2'000'000'000;
    compare({{low, high}, {low + width, high}, {low, high - width}, {low + width, high - width}},
            {{low + 1, high - 1}, {low + width / 2, high - width / 2}, {low, high}, {low, high - 1}});
    expect_overflow({{low, 0}}, {{high, 0}});
    expect_overflow({{0, 0}, {0, width + 1}}, {});

#ifndef SKIP_RUNTIME_CHECKS
    const sirkit::CountPointsInTriangle triangles(Points{{1, 2}, {3, 4}}, {});
    invalid_index(triangles, {2, 0, 0});
    invalid_index(triangles, {0, 2, 0});
    invalid_index(triangles, {0, 0, 2});
    invalid_index(sirkit::CountPointsInTriangle({}, {}), {0, 0, 0});
#endif

    std::mt19937 random(45924);
    for (int trial = 0; trial < 800; ++trial)
    {
        Points vertices(random() % 14), points(random() % 25);
        const auto fill = [&](Points &array)
        {
            const unsigned scale = trial % 2 == 0 ? 7 : 2'000'000'001;
            for (auto &[x, y] : array)
            {
                x = random() % scale;
                y = random() % scale;
            }
        };
        fill(vertices);
        fill(points);
        compare(vertices, points);
    }

    // 全頂点を同じ高さの3列に置き、水平辺・斜辺・三角形の退化が混ざる問い合わせを繰り返す。
    Points vertices, points;
    for (std::int64_t i = 0; i < 500; ++i)
    {
        vertices.emplace_back(i % 20, i % 3);
        points.emplace_back(i % 40, i % 3);
    }
    const sirkit::CountPointsInTriangle maximum(vertices, points);
    std::vector<std::array<std::size_t, 3>> queries;
    std::vector<std::size_t> expected;
    for (int i = 0; i < 1000; ++i)
    {
        const std::array<std::size_t, 3> q{random() % 500, random() % 500, random() % 500};
        queries.push_back(q);
        expected.push_back(reference(vertices[q[0]], vertices[q[1]], vertices[q[2]], points));
    }
    for (std::size_t i = 0; i < 1'000'000; ++i)
    {
        const auto [a, b, c] = queries[i % queries.size()];
        assert(maximum.count(a, b, c) == expected[i % queries.size()]);
    }
}
