// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/K_Shortest_Simple_Paths.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>

namespace
{
using Edge = sirkit::ShortestPathEdge;

void check(int n, const std::vector<Edge> &edges, int source, int target, std::size_t k, bool undirected)
{
    std::vector<std::uint64_t> distances;
    std::vector<bool> visited(n);
    auto dfs = [&](auto &self, int u, std::uint64_t distance) -> void
    {
        if (u == target)
        {
            distances.push_back(distance);
            return;
        }
        visited[u] = true;
        for (const auto &[from, to, weight] : edges)
        {
            if (weight > std::numeric_limits<std::uint64_t>::max() - distance)
            {
                continue;
            }
            if (from == u && !visited[to])
            {
                self(self, to, distance + weight);
            }
            else if (undirected && to == u && !visited[from])
            {
                self(self, from, distance + weight);
            }
        }
        visited[u] = false;
    };
    dfs(dfs, source, 0);
    std::ranges::sort(distances);
    distances.resize(std::min(k, distances.size()));

    const auto actual = sirkit::k_shortest_simple_paths(n, edges, source, target, k, undirected);
    assert(actual.size() == distances.size());
    std::set<std::vector<std::size_t>> distinct;
    for (std::size_t index = 0; index < actual.size(); ++index)
    {
        const auto &[distance, vertices, ids] = actual[index];
        assert(distance == distances[index]);
        assert(distinct.insert(ids).second);
        assert(vertices.size() == ids.size() + 1);
        assert(vertices.front() == source && vertices.back() == target);
        assert(std::set<int>(vertices.begin(), vertices.end()).size() == vertices.size());
        std::uint64_t sum = 0;
        for (std::size_t i = 0; i < ids.size(); ++i)
        {
            assert(ids[i] < edges.size());
            const auto [u, v, weight] = edges[ids[i]];
            assert(((vertices[i] == u && vertices[i + 1] == v) ||
                    (undirected && vertices[i] == v && vertices[i + 1] == u)));
            assert(weight <= std::numeric_limits<std::uint64_t>::max() - sum);
            sum += weight;
        }
        assert(sum == distance);
    }
}

template <typename Func>
void check_error(Func func)
{
    bool caught = false;
    try
    {
        func();
    }
    catch (const std::out_of_range &)
    {
        caught = true;
    }
    assert(caught);
}
} // namespace

int main()
{
    check(1, {}, 0, 0, 3, false);
    check(2, {{0, 1, 1}}, 0, 1, 0, false);
    check(2, {}, 0, 1, 4, false);
    check(3, {{0, 1, 0}, {0, 1, 0}, {1, 1, 0}, {1, 2, 0}, {0, 2, 0}}, 0, 2, 10, false);
    check(5, {{0, 1, 10}, {0, 2, 1}, {2, 1, 1}, {1, 3, 4}, {2, 3, 2},
              {0, 3, 12}, {3, 4, 100}, {2, 4, 150}},
          0, 4, 2, false);
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    check(4, {{0, 1, kMax}, {1, 3, 1}, {0, 2, kMax}, {2, 3, 0}, {0, 3, kMax}}, 0, 3, 6, true);

    std::mt19937 generator(759849);
    for (int test = 0; test < 1100; ++test)
    {
        const int n = static_cast<int>(generator() % 7) + 1;
        const int m = static_cast<int>(generator() % 17);
        const int source = static_cast<int>(generator() % n);
        const int target = static_cast<int>(generator() % n);
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            const int u = static_cast<int>(generator() % n);
            const int v = static_cast<int>(generator() % n);
            edges.push_back({u, v, generator() % 10});
        }
        check(n, edges, source, target, static_cast<std::size_t>(test % 12), test % 2 == 0);
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[source, target] : std::vector<std::pair<int, int>>{{-1, 0}, {2, 0}, {0, -1}, {0, 2}})
    {
        check_error([&] { (void)sirkit::k_shortest_simple_paths(2, {}, source, target, 1); });
    }
    for (const Edge edge : std::vector<Edge>{{-1, 0, 1}, {2, 0, 1}, {0, -1, 1}, {0, 2, 1}})
    {
        check_error([&] { (void)sirkit::k_shortest_simple_paths(2, std::span(&edge, 1), 0, 1, 1); });
    }
#endif
}
