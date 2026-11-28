// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Palindromic_Shortest_Paths.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::PalindromicPathEdge;

std::int64_t inward_search(int n, const std::vector<Edge> &arcs, int from, int to)
{
    std::vector steps(n, std::vector<int>(n, -1));
    std::queue<std::pair<int, int>> search;
    steps[from][to] = 0;
    search.emplace(from, to);
    constexpr int kMissing = std::numeric_limits<int>::max();
    int best = kMissing;
    while (!search.empty())
    {
        const auto [u, v] = search.front();
        search.pop();
        const int distance = 2 * steps[u][v];
        if (distance >= best)
        {
            break;
        }
        if (u == v)
        {
            best = distance;
            break;
        }
        for (const auto &left : arcs)
        {
            if (left.from != u)
            {
                continue;
            }
            if (left.to == v)
            {
                best = distance + 1;
            }
            for (const auto &right : arcs)
            {
                if (right.to == v && left.label == right.label && steps[left.to][right.from] == -1)
                {
                    steps[left.to][right.from] = steps[u][v] + 1;
                    search.emplace(left.to, right.from);
                }
            }
        }
    }
    return best == kMissing ? -1 : best;
}

void check(int n, const std::vector<Edge> &edges, bool undirected)
{
    std::vector<Edge> arcs = edges;
    if (undirected)
    {
        for (const auto &[u, v, label] : edges)
        {
            arcs.push_back({v, u, label});
        }
    }
    const sirkit::PalindromicShortestPaths paths(n, edges, undirected);
    assert(paths.size() == n);
    for (int u = 0; u < n; ++u)
    {
        for (int v = 0; v < n; ++v)
        {
            const auto expected = inward_search(n, arcs, u, v);
            assert(paths.distance(u, v) == expected);
            const auto actual = paths.path(u, v);
            if (expected == -1)
            {
                assert(!actual);
                continue;
            }
            assert(actual && actual->vertices.front() == u && actual->vertices.back() == v);
            assert(static_cast<std::int64_t>(actual->edges.size()) == expected);
            assert(actual->vertices.size() == actual->edges.size() + 1);
            for (std::size_t i = 0; i < actual->edges.size(); ++i)
            {
                const auto id = actual->edges[i];
                assert(id < edges.size());
                const auto &[from, to, label] = edges[id];
                const int a = actual->vertices[i];
                const int b = actual->vertices[i + 1];
                assert(((a == from && b == to) || (undirected && a == to && b == from)));
                assert(label == edges[actual->edges[actual->edges.size() - 1 - i]].label);
            }
        }
    }
}

template <typename Error, typename Func>
void check_error(Func func)
{
    bool found = false;
    try
    {
        func();
    }
    catch (const Error &)
    {
        found = true;
    }
    assert(found);
}
} // namespace

int main()
{
    check(0, {}, false);
    check(1, {}, true);
    check(3, {}, false);
    check(4, {{0, 1, 0}, {1, 2, 255}, {2, 3, 0}}, false);
    check(3, {{0, 0, 255}, {0, 1, 0}, {1, 2, 0}, {0, 1, 1}}, true);
    const std::vector<Edge> cycle{{0, 1, 42}, {1, 2, 42}, {2, 3, 42}, {3, 0, 42}};
    check(4, cycle, false);
    std::mt19937 random(45821);
    for (int test = 0; test < 620; ++test)
    {
        const int n = static_cast<int>(random() % 7) + 1;
        const int m = static_cast<int>(random() % 16);
        std::vector<Edge> edges;
        for (int edge = 0; edge < m; ++edge)
        {
            const int u = static_cast<int>(random() % n);
            const int v = static_cast<int>(random() % n);
            const auto label = static_cast<std::uint8_t>(random() % 4);
            edges.push_back({u, v, label});
        }
        check(n, edges, test % 2 == 0);
    }
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::length_error>([] { (void)sirkit::PalindromicShortestPaths(-1, {}); });
    check_error<std::length_error>(
        [] { (void)sirkit::PalindromicShortestPaths(std::numeric_limits<int>::max(), {}); });
    const sirkit::PalindromicShortestPaths isolated(2, {});
    check_error<std::out_of_range>([&] { (void)isolated.distance(-1, 0); });
    check_error<std::out_of_range>([&] { (void)isolated.path(0, 2); });
    for (const Edge edge : std::vector<Edge>{{-1, 0, 1}, {2, 0, 2}, {0, -1, 3}, {0, 2, 4}})
    {
        check_error<std::out_of_range>([&] { (void)sirkit::PalindromicShortestPaths(2, std::span(&edge, 1)); });
    }
#endif
}
