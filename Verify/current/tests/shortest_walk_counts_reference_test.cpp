// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Shortest_Walk_Counts.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <random>
#include <vector>

namespace
{
using Edge = sirkit::ShortestPathEdge;

struct ModuloFive
{
    unsigned count;
    ModuloFive &operator+=(const ModuloFive &other)
    {
        count = (count + other.count) % 5;
        return *this;
    }
};

void check(int n, const std::vector<Edge> &edges, int source, bool undirected)
{
    constexpr std::uint64_t kUnreachable = 1 << 24;
    std::vector dist(n, std::vector<std::uint64_t>(n, kUnreachable));
    for (int u = 0; u < n; ++u)
    {
        dist[u][u] = 0;
    }
    std::vector<Edge> arcs(edges);
    if (undirected)
    {
        for (const auto &[u, v, w] : edges)
        {
            if (u != v)
            {
                arcs.push_back({v, u, w});
            }
        }
    }
    for (const auto &[u, v, w] : arcs)
    {
        dist[u][v] = std::min(dist[u][v], w);
    }
    for (int k = 0; k < n; ++k)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                dist[u][v] = std::min(dist[u][v], dist[u][k] + dist[k][v]);
            }
        }
    }

    std::vector<Edge> exact;
    std::vector closure(n, std::vector<bool>(n));
    for (const auto &[u, v, w] : arcs)
    {
        if (dist[source][u] < kUnreachable && dist[source][u] + w == dist[source][v])
        {
            closure[u][v] = true;
            exact.push_back({u, v, w});
        }
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
    std::vector<std::uint64_t> layer(n), expected(n);
    layer[source] = expected[source] = 1;
    for (int length = 1; length < n; ++length)
    {
        std::vector<std::uint64_t> next(n);
        for (const auto &[u, v, w] : exact)
        {
            next[v] += layer[u];
        }
        layer = std::move(next);
        for (int v = 0; v < n; ++v)
        {
            expected[v] += layer[v];
        }
    }

    const auto answer = sirkit::count_shortest_walks(n, edges, source, undirected);
    const auto modular = sirkit::count_shortest_walks<ModuloFive>(n, edges, source, undirected);
    for (int v = 0; v < n; ++v)
    {
        bool infinite = false;
        for (int c = 0; c < n; ++c)
        {
            infinite |= closure[c][c] && (c == v || closure[c][v]);
        }
        assert(answer.distance[v].value_or(kUnreachable) == dist[source][v]);
        assert(answer.infinite[v] == infinite);
        const auto count = infinite ? 0 : expected[v];
        assert(answer.count[v] == count);
        assert(modular.count[v].count == count % 5);
    }
}
} // namespace

int main()
{
    check(2, {}, 0, false);
    check(4, {{0, 1, 1}, {0, 2, 1}, {1, 3, 1}, {2, 3, 1}, {0, 0, 1}}, 0, true);
    check(5, {{0, 1, 0}, {0, 2, 1}, {2, 1, 0}, {1, 3, 2}, {4, 4, 0}}, 0, false);
    check(5, {{0, 1, 0}, {1, 0, 0}, {0, 2, 3}, {1, 2, 3}, {2, 3, 3}, {4, 4, 0}}, 0, false);
    check(5, {{0, 1, 5}, {0, 2, 2}, {2, 1, 1}, {1, 3, 0}, {2, 2, 0}, {0, 4, 2}}, 0, false);
    std::mt19937 rng(72126);
    for (int trial = 0; trial < 1050; ++trial)
    {
        const int n = 1 + rng() % 8;
        const int m = rng() % 20;
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            const auto weight = trial % 3 == 0 ? std::uint64_t{1} : rng() % 5;
            edges.push_back({static_cast<int>(rng() % n), static_cast<int>(rng() % n), weight});
        }
        check(n, edges, rng() % n, (trial & 1) != 0);
    }

    constexpr auto kLimit = std::numeric_limits<std::uint64_t>::max();
    const std::array high{Edge{0, 1, kLimit}, Edge{1, 2, 0}, Edge{2, 3, 1}, Edge{0, 4, kLimit - 1}, Edge{4, 1, 1}};
    const auto answer = sirkit::count_shortest_walks(5, high, 0);
    assert(answer.distance[1] == kLimit && answer.distance[2] == kLimit);
    assert(answer.count[1] == 2 && answer.count[2] == 2 && answer.count[3] == 0);
    assert(!answer.distance[3] && !answer.infinite[2]);

#ifndef SKIP_RUNTIME_CHECKS
    int rejected = 0;
    for (const auto source : {-1, 1})
    {
        try
        {
            (void)sirkit::count_shortest_walks(1, {}, source);
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    for (const auto &edge : std::array{Edge{-1, 0, 0}, Edge{1, 0, 0}, Edge{0, -1, 0}, Edge{0, 1, 0}})
    {
        try
        {
            (void)sirkit::count_shortest_walks(1, std::span(&edge, 1), 0);
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    assert(rejected == 6);
#endif
}
