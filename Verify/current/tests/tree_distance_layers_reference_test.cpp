// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Distance_Layers.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

void check(int n, const std::vector<Edge> &edges)
{
    const sirkit::TreeDistanceLayers layers(n, edges);
    assert(layers.size() == static_cast<std::size_t>(n));
    std::vector<std::vector<int>> adjacent(n);
    for (const auto &[u, v] : edges)
    {
        adjacent[u].push_back(v);
        adjacent[v].push_back(u);
    }
    std::vector distance(n, std::vector<int>(n, -1));
    for (int source = 0; source < n; ++source)
    {
        auto &row = distance[source];
        std::vector<int> pending{source};
        row[source] = 0;
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const int u = pending[i];
            for (const int v : adjacent[u])
            {
                if (row[v] == -1)
                {
                    row[v] = row[u] + 1;
                    pending.push_back(v);
                }
            }
        }
    }
    for (int u = 0; u < n; ++u)
    {
        const auto &row = distance[u];
        const int longest = *std::ranges::max_element(row);
        assert(layers.eccentricity(u) == longest);
        assert(row[layers.farthest(u)] == longest);
        const auto [a, b] = layers.diameter_endpoints(u);
        assert(row[a] >= 0 && row[b] >= 0);
        int diameter = 0;
        for (int v = 0; v < n; ++v)
        {
            if (row[v] >= 0)
            {
                diameter = std::max(diameter, *std::ranges::max_element(distance[v]));
            }
        }
        assert(distance[a][b] == diameter);
        assert(layers.vertex_at_distance(u, -1) == -1);
        for (int k = 0; k <= n; ++k)
        {
            const int answer = layers.vertex_at_distance(u, k);
            if (k <= longest)
            {
                assert(answer >= 0 && row[answer] == k);
            }
            else
            {
                assert(answer == -1);
            }
        }
    }
}
} // namespace

int main()
{
    check(0, {});
    check(1, {});
    check(5, {{1, 0}, {0, 2}, {3, 4}});
    std::mt19937 rng(19743);
    for (int trial = 0; trial < 750; ++trial)
    {
        const int n = 1 + rng() % 32;
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            if (rng() % 5 != 0)
            {
                edges.emplace_back(rng() % v, v);
            }
        }
        std::ranges::shuffle(edges, rng);
        check(n, edges);
    }

#ifndef SKIP_RUNTIME_CHECKS
    const sirkit::TreeDistanceLayers layers(1, {});
    int rejected = 0;
    for (const int invalid : {-1, 1})
    {
        try
        {
            (void)layers.vertex_at_distance(invalid, 0);
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    try
    {
        const std::array cycle{Edge{0, 1}, Edge{1, 2}, Edge{2, 0}};
        (void)sirkit::TreeDistanceLayers(3, cycle);
    }
    catch (const std::invalid_argument &)
    {
        ++rejected;
    }
    assert(rejected == 3);
#endif
}
