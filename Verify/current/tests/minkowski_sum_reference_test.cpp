// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_MINKOWSKI_SUM_DEFINED
#include "../../../Geometry/Minkowski_Sum.cpp"
#endif
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Point = std::pair<std::int64_t, std::int64_t>;
using Polygon = std::vector<Point>;

Polygon reference_hull(const Polygon &points)
{
    if (points.empty())
    {
        return {};
    }
    const auto first = *std::ranges::min_element(points, {}, [](Point p)
                                                 {
                                                     return std::pair(p.second, p.first);
                                                 });
    Polygon hull;
    auto current = first;
    do
    {
        hull.push_back(current);
        auto next = current;
        for (const auto &candidate : points)
        {
            const auto dx = next.first - current.first;
            const auto dy = next.second - current.second;
            const auto x = candidate.first - current.first;
            const auto y = candidate.second - current.second;
            const auto cross = dx * y - dy * x;
            if (cross < 0 || (cross == 0 && x * x + y * y > dx * dx + dy * dy))
            {
                next = candidate;
            }
        }
        current = next;
    } while (current != first);
    return hull;
}

Polygon reference_sum(const Polygon &a, const Polygon &b)
{
    Polygon points;
    for (const auto &[x, y] : a)
    {
        for (const auto &[u, v] : b)
        {
            points.emplace_back(x + u, y + v);
        }
    }
    return reference_hull(points);
}

Polygon random_boundary(std::mt19937 &random)
{
    Polygon points(random() % 17);
    for (auto &[x, y] : points)
    {
        x = 6 * (static_cast<std::int64_t>(random() % 13) - 6);
        y = 6 * (static_cast<std::int64_t>(random() % 13) - 6);
    }
    auto hull = reference_hull(points);
    if (hull.empty())
    {
        return {};
    }
    Polygon boundary;
    for (std::size_t i = 0; i < hull.size(); ++i)
    {
        const auto p = hull[i], q = hull[(i + 1) % hull.size()];
        boundary.push_back(p);
        if (random() % 2 == 0)
        {
            boundary.push_back(p);
        }
        if (random() % 2 == 0)
        {
            boundary.emplace_back((p.first + q.first) / 2, (p.second + q.second) / 2);
        }
    }
    if (random() % 2 == 0)
    {
        std::ranges::reverse(boundary);
    }
    std::ranges::rotate(boundary, boundary.begin() + random() % boundary.size());
    if (random() % 2 == 0)
    {
        boundary.push_back(boundary.front());
    }
    return boundary;
}

void expect_overflow(const Polygon &a, const Polygon &b)
{
    bool failed = false;
    try
    {
        (void)sirkit::minkowski_sum(a, b);
    }
    catch (const std::overflow_error &)
    {
        failed = true;
    }
    assert(failed);
}

#ifndef SKIP_RUNTIME_CHECKS
void expect_invalid(const Polygon &a)
{
    bool failed = false;
    try
    {
        (void)sirkit::minkowski_sum(a, std::array{Point{0, 0}});
    }
    catch (const std::invalid_argument &)
    {
        failed = true;
    }
    assert(failed);
}
#endif
} // namespace

int main()
{
    assert(sirkit::minkowski_sum({}, {}).empty());
    assert(sirkit::minkowski_sum(std::array{Point{0, 0}}, {}).empty());
    assert((sirkit::minkowski_sum(std::array{Point{1, 2}}, std::array{Point{3, 4}}) == Polygon{{4, 6}}));
    assert((sirkit::minkowski_sum(Polygon{{2, 2}, {0, 0}, {1, 1}, {-1, -1}, {2, 2}}, std::array{Point{0, 0}}) ==
            Polygon{{-1, -1}, {2, 2}}));
    assert((sirkit::minkowski_sum(Polygon{{1, 0}, {-1, 0}}, Polygon{{0, 2}, {0, -2}}) ==
            Polygon{{-1, -2}, {1, -2}, {1, 2}, {-1, 2}}));

    constexpr auto hi = std::numeric_limits<std::int64_t>::max();
    constexpr auto lo = std::numeric_limits<std::int64_t>::min();
    constexpr std::int64_t width = std::numeric_limits<std::uint32_t>::max();
    const Polygon low_rectangle{{lo, 0}, {lo + width, 0}, {lo + width, width}, {lo, width}};
    const Polygon high_rectangle{{hi - width, -width}, {hi, -width}, {hi, 0}, {hi - width, 0}};
    const Polygon combined{{-width - 1, -width}, {width - 1, -width}, {width - 1, width}, {-width - 1, width}};
    assert(sirkit::minkowski_sum(low_rectangle, high_rectangle) == combined);
    const Polygon anti_diagonal{{width, 0}, {0, width}};
    const Polygon diagonal{{0, 0}, {width, width}};
    assert((sirkit::minkowski_sum(diagonal, anti_diagonal) ==
            Polygon{{width, 0}, {2 * width, width}, {width, 2 * width}, {0, width}}));
    assert((sirkit::minkowski_sum(std::array{Point{hi, lo}}, std::array{Point{0, 0}}) == Polygon{{hi, lo}}));
    expect_overflow({{hi, 0}}, {{1, 0}});
    expect_overflow({{0, lo}}, {{0, -1}});
    expect_overflow({{lo, 0}, {hi, 0}}, {{0, 0}});
    expect_overflow({{0, 0}, {0, width + 1}}, {{0, 0}});

#ifndef SKIP_RUNTIME_CHECKS
    expect_invalid({{0, 0}, {2, 2}, {0, 2}, {2, 0}});
    expect_invalid({{0, 0}, {1, 0}, {3, 2}, {2, 3}, {1, 0}});
    expect_invalid({{0, 0}, {4, 0}, {1, 1}, {4, 4}, {0, 4}});
#endif

    std::mt19937 random(271414);
    for (int trial = 0; trial < 5000; ++trial)
    {
        const auto a = random_boundary(random);
        const auto b = random_boundary(random);
        const auto expected = reference_sum(a, b);
        assert(sirkit::minkowski_sum(a, b) == expected);
        assert(sirkit::minkowski_sum(b, a) == expected);
    }

    Polygon chain, doubled;
    for (std::int64_t x = -25000; x <= 25000; ++x)
    {
        chain.emplace_back(x, x * x);
        doubled.emplace_back(2 * x, 2 * x * x);
    }
    std::ranges::rotate(doubled, doubled.begin() + doubled.size() / 2);
    assert(sirkit::minkowski_sum(chain, chain) == doubled);
}
