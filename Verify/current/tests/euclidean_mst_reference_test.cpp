// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_EUCLIDEAN_MST_DEFINED
#include "../../../Geometry/Euclidean_MST.cpp"
#endif
#include <algorithm>
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
using Points = std::vector<Point>;

std::uint64_t distance(Point a, Point b)
{
    const auto dx = a.first - b.first, dy = a.second - b.second;
    return static_cast<std::uint64_t>(dx * dx + dy * dy);
}

std::uint64_t prim(const Points &points)
{
    if (points.empty())
    {
        return 0;
    }
    std::vector<std::uint64_t> best(points.size(), std::numeric_limits<std::uint64_t>::max());
    std::vector<bool> selected(points.size());
    best[0] = 0;
    std::uint64_t total = 0;
    for (std::size_t i = 0; i < points.size(); ++i)
    {
        auto next = points.size();
        for (std::size_t j = 0; j < points.size(); ++j)
        {
            if (!selected[j] && (next == points.size() || best[j] < best[next]))
            {
                next = j;
            }
        }
        total += best[next];
        selected[next] = true;
        for (std::size_t j = 0; j < points.size(); ++j)
        {
            best[j] = std::min(best[j], distance(points[next], points[j]));
        }
    }
    return total;
}

void compare(const Points &points)
{
    const auto tree = sirkit::euclidean_mst(points);
    assert(tree.size() == points.size() - !points.empty());
    std::vector<std::vector<std::size_t>> incident(points.size());
    std::uint64_t total = 0;
    for (const auto &e : tree)
    {
        assert(e.from != e.to && e.from < points.size() && e.to < points.size());
        assert(e.distance_squared == distance(points[e.from], points[e.to]));
        total += e.distance_squared;
        incident[e.from].push_back(e.to);
        incident[e.to].push_back(e.from);
    }
    assert(total == prim(points));

    if (points.empty())
    {
        return;
    }
    std::vector<bool> reached(points.size());
    reached[0] = true;
    std::vector<std::size_t> queue{0};
    for (std::size_t i = 0; i < queue.size(); ++i)
    {
        for (const auto next : incident[queue[i]])
        {
            if (!reached[next])
            {
                reached[next] = true;
                queue.push_back(next);
            }
        }
    }
    assert(queue.size() == points.size());
}
} // namespace

int main()
{
    compare({});
    compare({{1, 1}});
    compare({{1, 1}, {0, 0}});
    compare({{2, -2}, {0, 0}, {1, -1}});
    compare({{2, 2}, {1, 2}, {2, 1}});
    compare({{0, 0}, {1, 0}, {1, 1}, {0, 1}});
    compare({{0, 0}, {0, 0}, {0, 0}, {0, 0}});
    compare({{5, 0}, {4, 3}, {3, 4}, {0, 5}, {-4, 3}, {-5, 0}, {-3, -4}, {0, -5}, {3, -4}});
    constexpr auto lo = std::numeric_limits<std::int64_t>::min();
    constexpr auto hi = std::numeric_limits<std::int64_t>::max();
    compare({{hi, lo}, {hi - 25'000, lo}, {hi, lo + 25'000}, {hi - 25'000, lo + 25'000}, {hi - 1, lo + 12'500}, {hi - 12'500, lo + 12'500}, {hi - 1, lo + 12'500}});

    bool threw = false;
    try
    {
        (void)sirkit::euclidean_mst(Points{{lo, 0}, {hi, 0}});
    }
    catch (const std::overflow_error &)
    {
        threw = true;
    }
    assert(threw);

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
        compare(points);
    }

    std::mt19937 random(440550);
    for (int trial = 0; trial < 6500; ++trial)
    {
        Points points(random() % 57);
        const unsigned limit_x = trial % 3 == 0 ? 5 : (trial % 3 == 1 ? 11 : 25'001);
        const unsigned limit_y = trial % 5 == 0 ? 5 : (trial % 5 == 1 ? 11 : 25'001);
        for (auto &[x, y] : points)
        {
            x = random() % limit_x;
            y = random() % limit_y;
        }
        compare(points);
    }

    Points grid;
    for (std::int64_t x = 0; x < 160; ++x)
    {
        for (std::int64_t y = 0; y < 160; ++y)
        {
            grid.emplace_back(x, y);
        }
    }
    const auto tree = sirkit::euclidean_mst(grid);
    assert(tree.size() + 1 == grid.size());
    assert(std::ranges::all_of(tree, [](const auto &e)
                               {
                                   return e.distance_squared == 1;
                               }));
}
