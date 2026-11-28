// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Walk_Weight_Ranges.cpp"

#include <cassert>
#include <limits>
#include <random>
#include <vector>

namespace
{
using Edge = sirkit::WalkWeightEdge;

void check(int n, const std::vector<Edge> &edges)
{
    constexpr std::int64_t kInfinity = std::numeric_limits<std::int64_t>::max() / 2;
    std::vector low(n, std::vector<std::int64_t>(n, kInfinity));
    std::vector high(n, std::vector<std::int64_t>(n, -kInfinity));
    for (int u = 0; u < n; ++u)
    {
        low[u][u] = high[u][u] = 0;
    }
    for (const auto &[u, v, weight] : edges)
    {
        low[u][v] = std::min<std::int64_t>(low[u][v], weight);
        high[u][v] = std::max<std::int64_t>(high[u][v], weight);
    }
    for (int k = 0; k < n; ++k)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                if (low[u][k] != kInfinity && low[k][v] != kInfinity)
                {
                    low[u][v] = std::min(low[u][v], low[u][k] + low[k][v]);
                    high[u][v] = std::max(high[u][v], high[u][k] + high[k][v]);
                }
            }
        }
    }

    for (int source = 0; source < n; ++source)
    {
        const auto answer = sirkit::walk_weight_ranges(n, edges, source);
        assert(answer.size() == static_cast<std::size_t>(n));
        for (int vertex = 0; vertex < n; ++vertex)
        {
            const bool reachable = low[source][vertex] != kInfinity;
            bool unbounded = false;
            for (int k = 0; k < n; ++k)
            {
                unbounded |= low[source][k] != kInfinity && low[k][vertex] != kInfinity &&
                             (low[k][k] != 0 || high[k][k] != 0);
            }
            const auto &range = answer[vertex];
            assert(range.reachable == reachable && range.unbounded == unbounded);
            const bool finite = reachable && !unbounded;
            assert(range.minimum == (finite ? low[source][vertex] : 0));
            assert(range.maximum == (finite ? high[source][vertex] : 0));
        }
    }
}
} // namespace

int main()
{
    check(1, {});
    check(2, {{1, 1, 0}, {0, 0, -1}});
    check(5, {{0, 1, 4}, {1, 0, -4}, {0, 2, 7}, {1, 2, 5}, {2, 3, -11}, {3, 2, 11}, {2, 4, 6}});
    check(5, {{0, 1, 1}, {1, 2, 2}, {2, 2, -1}, {2, 4, 0}, {0, 3, 2}, {3, 4, 3}});
    constexpr auto kBottom = std::numeric_limits<std::int32_t>::min();
    constexpr auto kTop = std::numeric_limits<std::int32_t>::max();
    check(6, {{0, 1, kBottom}, {1, 2, kBottom}, {2, 3, kTop}, {3, 4, kTop}, {0, 4, 0}, {4, 5, kBottom}});
    check(5, {{1, 0, kTop}, {0, 1, -kTop}, {1, 2, kBottom}, {2, 3, kTop}, {3, 4, kTop}});

    std::mt19937 rng(603399);
    for (int trial = 0; trial < 1100; ++trial)
    {
        const int n = 1 + rng() % 8;
        const int m = rng() % 20;
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            edges.push_back({static_cast<int>(rng() % n), static_cast<int>(rng() % n), static_cast<int>(rng() % 15) - 7});
        }
        check(n, edges);
    }

    sirkit::scc_graph scc(3);
    scc.add_edge(0, 1);
    assert(scc.build() == 3 && scc.condensation()[0].size() <= 1);

#ifndef SKIP_RUNTIME_CHECKS
    int rejected = 0;
    for (const int source : {-1, 0})
    {
        try
        {
            (void)sirkit::walk_weight_ranges(0, {}, source);
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    const std::vector<Edge> invalid{{0, 1, 0}};
    try
    {
        (void)sirkit::walk_weight_ranges(1, invalid, 0);
    }
    catch (const std::out_of_range &)
    {
        ++rejected;
    }
    assert(rejected == 3);
#endif
}
