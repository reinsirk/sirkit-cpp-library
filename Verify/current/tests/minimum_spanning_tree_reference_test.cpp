// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_MINIMUM_SPANNING_TREE_TEST_SOURCE
#define SIRKIT_MINIMUM_SPANNING_TREE_TEST_SOURCE "../../../Graph/Minimum_Spanning_Tree.cpp"
#endif
#include SIRKIT_MINIMUM_SPANNING_TREE_TEST_SOURCE
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
using Edge = sirkit::SpanningTreeEdge;

// 各成分を隣接行列のPrim法で独立に求める。
std::pair<int, std::int64_t> Prim(int n, std::span<const Edge> edges)
{
    constexpr auto kInf = std::numeric_limits<std::int64_t>::max();
    std::vector<std::vector<std::int64_t>> weights(n, std::vector<std::int64_t>(n, kInf));
    for (const auto &[u, v, w] : edges)
    {
        weights[u][v] = weights[v][u] = std::min(weights[u][v], w);
    }
    std::vector<bool> used(n);
    std::vector<std::int64_t> cost(n, kInf);
    int components = 0;
    std::int64_t result = 0;
    for (int i = 0; i < n; ++i)
    {
        int vertex = -1;
        for (int v = 0; v < n; ++v)
        {
            if (!used[v] && (vertex < 0 || cost[v] < cost[vertex]))
            {
                vertex = v;
            }
        }
        used[vertex] = true;
        if (cost[vertex] == kInf)
        {
            ++components;
        }
        else
        {
            result += cost[vertex];
        }
        for (int v = 0; v < n; ++v)
        {
            cost[v] = std::min(cost[v], weights[vertex][v]);
        }
    }
    return {n - components, result};
}

void Check(int n, std::span<const Edge> edges)
{
    const auto chosen = sirkit::minimum_spanning_forest(n, edges);
    const auto [count, total] = Prim(n, edges);
    assert(chosen.size() == static_cast<std::size_t>(count));
    std::vector<int> color(n);
    std::iota(color.begin(), color.end(), 0);
    std::int64_t cost = 0;
    for (auto id : chosen)
    {
        assert(id < edges.size());
        const auto &[u, v, w] = edges[id];
        const int before = color[u], after = color[v];
        assert(before != after);
        std::replace(color.begin(), color.end(), before, after);
        cost += w;
    }
    assert(cost == total);
}

#ifndef SKIP_RUNTIME_CHECKS
template <class Exception>
bool Rejects(int n, std::span<const Edge> edges)
{
    try
    {
        sirkit::minimum_spanning_forest(n, edges);
    }
    catch (const Exception &)
    {
        return true;
    }
    return false;
}
#endif
} // namespace

int main()
{
    std::mt19937 random(66461);
    for (int n = 0; n <= 18; ++n)
    {
        for (int repetition = 0; repetition < 260; ++repetition)
        {
            std::vector<Edge> edges;
            if (n != 0)
            {
                const int m = repetition % (4 * n + 1);
                for (int i = 0; i < m; ++i)
                {
                    const auto u = static_cast<int>(random() % n), v = static_cast<int>(random() % n);
                    edges.push_back({u, v, static_cast<int>(random() % 21) - 10});
                }
            }
            Check(n, edges);
        }
    }
    const std::vector<Edge> ties{{2, 2, 0}, {2, 1, 4}, {0, 1, 4}, {0, 2, 4}, {3, 3, -1}};
    assert((sirkit::minimum_spanning_forest(4, ties) == std::vector<std::size_t>{1, 2}));
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    const std::vector<Edge> extremes{{0, 1, kMax}, {0, 1, kMin}, {1, 2, kMax}};
    assert((sirkit::minimum_spanning_forest(3, extremes) == std::vector<std::size_t>{1, 2}));
    std::vector<Edge> large;
    for (int i = 0; i < 7021; ++i)
    {
        large.push_back({static_cast<int>(random() % 17), static_cast<int>(random() % 17),
                         static_cast<std::int64_t>(random() % 2000000001) - 1000000000});
    }
    Check(17, large);
    for (auto &edge : large)
    {
        edge.weight = -7;
    }
    Check(17, large);
    large.assign(1024, {0, 0, 0});
    std::copy(extremes.begin(), extremes.end(), large.begin());
    assert((sirkit::minimum_spanning_forest(3, large) == std::vector<std::size_t>{1, 2}));
    // 辺番号に10/11ビット必要な場合に、残る重みビット数の境界をまたぐ。
    for (std::size_t edge_count : {1024, 1025})
    {
        const auto boundary = std::int64_t{1} << (edge_count == 1024 ? 54 : 53);
        for (const auto weight_range : {boundary - 1, boundary})
        {
            for (const auto lowest : {kMin, std::int64_t{0}, kMax - weight_range})
            {
                const auto highest = lowest + weight_range;
                large.assign(edge_count, {0, 0, lowest});
                large[0] = {0, 1, highest};
                large[edge_count - 2] = {0, 1, lowest};
                large.back() = {1, 2, highest};
                const std::vector<std::size_t> expected{edge_count - 2, edge_count - 1};
                assert(sirkit::minimum_spanning_forest(3, large) == expected);
            }
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    assert(Rejects<std::invalid_argument>(-1, {}));
    for (const Edge &invalid : {Edge{-1, 0, 0}, Edge{2, 0, 0}, Edge{0, -1, 0}, Edge{0, 2, 0}})
    {
        assert(Rejects<std::out_of_range>(2, std::span{&invalid, 1}));
    }
#endif
}
