// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_MANHATTAN_MST_TEST_SOURCE
#define SIRKIT_MANHATTAN_MST_TEST_SOURCE "../../../Geometry/Manhattan_MST.cpp"
#endif
#include SIRKIT_MANHATTAN_MST_TEST_SOURCE
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Point = std::pair<std::int64_t, std::int64_t>;

std::int64_t Distance(Point a, Point b)
{
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

std::int64_t DensePrim(std::span<const Point> points)
{
    if (points.empty())
    {
        return 0;
    }
    const int n = static_cast<int>(points.size());
    std::vector<bool> chosen(n);
    std::vector<std::int64_t> distance(n, std::numeric_limits<std::int64_t>::max());
    distance.front() = 0;
    std::int64_t cost = 0;
    for (int i = 0; i < n; ++i)
    {
        int next = -1;
        for (int j = 0; j < n; ++j)
        {
            if (!chosen[j] && (next < 0 || distance[j] < distance[next]))
            {
                next = j;
            }
        }
        cost += distance[next];
        chosen[next] = true;
        for (int j = 0; j < n; ++j)
        {
            distance[j] = std::min(distance[j], Distance(points[next], points[j]));
        }
    }
    return cost;
}

void Check(std::span<const Point> points)
{
    const int n = static_cast<int>(points.size());
    const auto result = sirkit::manhattan_mst(points);
    assert(result.cost == DensePrim(points));
    assert(result.edges.size() == static_cast<std::size_t>(std::max(n - 1, 0)));
    if (n == 0)
    {
        return;
    }
    std::int64_t cost = 0;
    std::vector<std::vector<int>> adjacency(n);
    for (const auto &[u, v] : result.edges)
    {
        assert(0 <= u && u < n && 0 <= v && v < n);
        adjacency[u].push_back(v);
        adjacency[v].push_back(u);
        cost += Distance(points[u], points[v]);
    }
    std::vector<bool> visited(n);
    std::vector<int> queue{0};
    visited[0] = true;
    for (std::size_t i = 0; i < queue.size(); ++i)
    {
        for (int to : adjacency[queue[i]])
        {
            if (!visited[to])
            {
                visited[to] = true;
                queue.push_back(to);
            }
        }
    }
    assert(queue.size() == points.size());
    assert(cost == result.cost);
}

template <class Exception, class Operation>
bool Throws(Operation operation)
{
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        return true;
    }
    return false;
}
} // namespace

int main()
{
    std::mt19937 random(87430);
    for (int n = 0; n <= 35; ++n)
    {
        for (int repetition = 0; repetition < 120; ++repetition)
        {
            std::vector<Point> points(n);
            const int period = repetition % 19 + 1;
            for (auto &[x, y] : points)
            {
                x = static_cast<int>(random() % period) - period / 2;
                y = static_cast<int>(random() % period) - period / 2;
            }
            Check(points);
        }
    }
    sirkit::detail::ManhattanRankSet active(4097);
    assert(active.lower_bound(0) == 4097);
    active.insert(4096);
    assert(active.lower_bound(0) == 4096);
    active.insert(66);
    active.insert(70);
    assert(active.lower_bound(65) == 66);
    active.erase(66);
    assert(active.lower_bound(65) == 70);
    active.erase(70);
    assert(active.lower_bound(4095) == 4096);
    active.erase(4096);
    assert(active.lower_bound(4096) == 4097);
    assert(active.lower_bound(8192) == 4097);
    std::vector<Point> diagonal;
    for (int i = 0; i < 257; ++i)
    {
        diagonal.emplace_back(i, -i);
    }
    Check(diagonal);
    std::vector<Point> duplicate(400, {3, 5});
    Check(duplicate);
    std::vector<Point> spread(500);
    for (auto &[x, y] : spread)
    {
        x = random() % 1000000;
        y = random() % 1000000;
    }
    Check(spread);
    Check(std::array<Point, 2>{{{0, 0}, {2, 1}}});
    constexpr auto kLimit = std::numeric_limits<std::int64_t>::max() / 4;
    Check(std::array<Point, 2>{{{-kLimit, -kLimit}, {kLimit, kLimit}}});
    // 十分な候補辺を作り、63ビットを使う距離を基数ソートへ通す。
    std::vector<Point> distant(600, {-kLimit, -kLimit});
    std::fill(distant.begin() + 300, distant.end(), Point{kLimit, kLimit});
    Check(distant);
    const std::array<Point, 4> corners{{{-kLimit, -kLimit}, {-kLimit, kLimit}, {kLimit, -kLimit}, {kLimit, kLimit}}};
    assert(Throws<std::overflow_error>([&]
                                       {
                                           sirkit::manhattan_mst(corners);
                                       }));
    assert(Throws<std::length_error>([]
                                     {
                                         sirkit::detail::CheckedManhattanMSTSize(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
                                     }));
#ifndef SKIP_RUNTIME_CHECKS
    for (const Point &invalid : {Point{-kLimit - 1, 0}, Point{kLimit + 1, 0}, Point{0, -kLimit - 1}, Point{0, kLimit + 1}})
    {
        assert(Throws<std::out_of_range>([&]
                                         {
                                             sirkit::manhattan_mst(std::array{invalid});
                                         }));
    }
#endif
}
