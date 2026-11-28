// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Bridge_Block_Orientation.cpp"

#include <array>
#include <cassert>
#include <random>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

std::vector<bool> connected(int n, const std::vector<Edge> &edges, int source, const std::vector<bool> &removed)
{
    std::vector<bool> seen(n);
    std::vector<int> pending{source};
    seen[source] = true;
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const int u = pending[i];
        for (std::size_t e = 0; e < edges.size(); ++e)
        {
            if (removed[e])
            {
                continue;
            }
            const auto [a, b] = edges[e];
            const int v = a == u ? b : b == u ? a
                                              : -1;
            if (v >= 0 && !seen[v])
            {
                seen[v] = true;
                pending.push_back(v);
            }
        }
    }
    return seen;
}

void check(int n, const std::vector<Edge> &edges)
{
    const auto orientation = sirkit::orient_bridge_blocks(n, edges);
    assert(orientation.size() == edges.size());
    std::vector<bool> removed(edges.size()), bridges(edges.size());
    std::vector closure(n, std::vector<bool>(n));
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v] = edges[id];
        const auto [from, to] = orientation[id];
        assert((Edge{from, to} == edges[id] || Edge{to, from} == edges[id]));
        removed[id] = true;
        bridges[id] = !connected(n, edges, u, removed)[v];
        removed[id] = false;
        closure[from][to] = true;
    }
    for (int u = 0; u < n; ++u)
    {
        closure[u][u] = true;
    }
    for (int k = 0; k < n; ++k)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                closure[u][v] = closure[u][v] || (closure[u][k] && closure[k][v]);
            }
        }
    }
    for (int u = 0; u < n; ++u)
    {
        const auto expected = connected(n, edges, u, bridges);
        for (int v = 0; v < n; ++v)
        {
            assert((closure[u][v] && closure[v][u]) == expected[v]);
        }
    }
}
} // namespace

int main()
{
    check(0, {});
    check(4, {});
    check(9, {{0, 1}, {1, 2}, {2, 0}, {2, 3}, {4, 3}, {4, 3}, {5, 5}, {6, 7}, {7, 8}});
    std::mt19937 rng(3286521);
    for (int trial = 0; trial < 1100; ++trial)
    {
        const int n = 1 + rng() % 9;
        const int m = rng() % 20;
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            edges.emplace_back(rng() % n, rng() % n);
        }
        check(n, edges);
    }

    constexpr int kLong = 200000;
    std::vector<Edge> long_cycle;
    long_cycle.reserve(kLong);
    for (int u = 0; u + 1 < kLong; ++u)
    {
        long_cycle.emplace_back(u, u + 1);
    }
    long_cycle.emplace_back(kLong - 1, 0);
    assert(sirkit::orient_bridge_blocks(kLong, long_cycle) == long_cycle);

#ifndef SKIP_RUNTIME_CHECKS
    int rejected = 0;
    try
    {
        (void)sirkit::orient_bridge_blocks(-1, {});
    }
    catch (const std::invalid_argument &)
    {
        ++rejected;
    }
    for (const auto &edge : std::array{Edge{-1, 0}, Edge{1, 0}, Edge{0, -1}, Edge{0, 1}})
    {
        try
        {
            (void)sirkit::orient_bridge_blocks(1, std::span(&edge, 1));
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    assert(rejected == 5);
#endif
}
