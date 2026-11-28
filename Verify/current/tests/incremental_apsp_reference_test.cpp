// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Incremental_APSP.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <random>
#include <vector>

namespace
{
using Edge = sirkit::IncrementalAPSPEdge;
void check(int n, const std::vector<Edge> &initial, const std::vector<Edge> &updates, bool undirected)
{
    constexpr std::int64_t kMissing = std::int64_t{1} << 50;
    auto arcs = initial;
    sirkit::IncrementalAPSP paths(n, initial, undirected);
    assert(paths.size() == static_cast<std::size_t>(n));
    for (std::size_t step = 0; step <= updates.size(); ++step)
    {
        if (step != 0)
        {
            const auto edge = updates[step - 1];
            arcs.push_back(edge);
            paths.add_edge(edge.from, edge.to, edge.weight);
        }
        std::vector matrix(n, std::vector<std::int64_t>(n, kMissing));
        for (int v = 0; v < n; ++v)
        {
            matrix[v][v] = 0;
        }
        for (const auto &[u, v, w] : arcs)
        {
            matrix[u][v] = std::min<std::int64_t>(matrix[u][v], w);
            if (undirected)
            {
                matrix[v][u] = std::min<std::int64_t>(matrix[v][u], w);
            }
        }
        for (int k = 0; k < n; ++k)
        {
            for (int u = 0; u < n; ++u)
            {
                for (int v = 0; v < n; ++v)
                {
                    matrix[u][v] = std::min(matrix[u][v], matrix[u][k] + matrix[k][v]);
                }
            }
        }
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                assert(paths.distance(u, v).value_or(kMissing) == matrix[u][v]);
            }
        }
    }
}
} // namespace

int main()
{
    check(0, {}, {}, false);
    check(1, {}, {{0, 0, 1}}, true);
    check(5, {{0, 1, 0}, {1, 2, 7}, {1, 2, 5}}, {{2, 3, 4}, {0, 3, 2}, {0, 3, 5}, {3, 4, 0}}, true);
    constexpr auto kWeight = std::numeric_limits<std::uint32_t>::max();
    check(6, {{0, 1, kWeight}, {1, 2, kWeight}}, {{2, 3, kWeight}, {3, 0, kWeight}, {0, 3, 2}, {4, 5, 0}}, false);
    std::mt19937 rng(225774);
    for (int trial = 0; trial < 720; ++trial)
    {
        const int n = 1 + rng() % 9;
        std::vector<Edge> initial, updates;
        const auto random_edge = [&]
        {
            return Edge{static_cast<int>(rng() % n), static_cast<int>(rng() % n), static_cast<std::uint32_t>(rng() % 31)};
        };
        const unsigned initially = rng() % 12;
        for (unsigned i = 0; i < initially; ++i)
        {
            initial.push_back(random_edge());
        }
        for (int i = 0; i < 20; ++i)
        {
            updates.push_back(random_edge());
        }
        check(n, initial, updates, (trial & 1) != 0);
    }

    int rejected = 0;
    try
    {
        (void)sirkit::IncrementalAPSP(-1);
    }
    catch (const std::invalid_argument &)
    {
        ++rejected;
    }
    assert(rejected == 1);
#ifndef SKIP_RUNTIME_CHECKS
    sirkit::IncrementalAPSP paths(1);
    for (const auto &edge : std::array{Edge{-1, 0, 0}, Edge{1, 0, 0}, Edge{0, -1, 0}, Edge{0, 1, 0}})
    {
        try
        {
            (void)sirkit::IncrementalAPSP(1, std::span(&edge, 1));
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    try
    {
        paths.add_edge(-1, 0, 1);
    }
    catch (const std::out_of_range &)
    {
        ++rejected;
    }
    try
    {
        (void)paths.distance(0, 1);
    }
    catch (const std::out_of_range &)
    {
        ++rejected;
    }
    assert(rejected == 7);
#endif
}
