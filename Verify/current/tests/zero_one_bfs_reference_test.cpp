// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Zero_One_BFS.cpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <utility>
#include <vector>

int main()
{
    std::mt19937 random(94);
    for (int n = 1; n <= 25; ++n)
    {
        for (int trial = 0; trial < 100; ++trial)
        {
            std::vector<std::vector<std::pair<int, int>>> edges(n);
            std::vector<std::vector<int>> reference(n, std::vector<int>(n, n + 1));
            for (int a = 0; a < n; ++a)
            {
                reference[a][a] = 0;
                for (int b = 0; b < n; ++b)
                {
                    if (random() % 6 == 0)
                    {
                        const int w = random() % 2;
                        edges[a].emplace_back(b, w);
                        reference[a][b] = std::min(reference[a][b], w);
                    }
                }
            }
            for (int k = 0; k < n; ++k)
            {
                for (int i = 0; i < n; ++i)
                {
                    for (int j = 0; j < n; ++j)
                    {
                        reference[i][j] = std::min(reference[i][j], reference[i][k] + reference[k][j]);
                    }
                }
            }
            const int source = random() % n;
            const auto result = sirkit::zero_one_bfs(n, source, [&](int v, const auto &visit)
                                                     {
                                                         for (auto [to, w] : edges[v])
                                                         {
                                                             visit(to, w);
                                                         }
                                                     });
            assert(result.distance[source] == 0 && result.parent[source] == source);
            for (int v = 0; v < n; ++v)
            {
                const auto expected = reference[source][v];
                if (expected > n)
                {
                    assert(result.distance[v] == -1 && result.parent[v] == -1);
                    continue;
                }
                assert(result.distance[v] == expected);
                int count = 0;
                for (int next = v; next != source;)
                {
                    const int parent = result.parent[next];
                    assert(parent >= 0 && parent < n);
                    assert(std::ranges::any_of(edges[parent], [&](auto edge)
                                               {
                                                   return edge.first == next &&
                                                          result.distance[parent] + edge.second == result.distance[next];
                                               }));
                    next = parent;
                    assert(++count < n);
                }
            }
        }
    }
    const auto neighbors = [](int, const auto &)
    {
    };
    try
    {
        sirkit::zero_one_bfs(-1, 0, neighbors);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto start : {-1, 1})
    {
        try
        {
            sirkit::zero_one_bfs(1, start, neighbors);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
    for (const auto &[to, w] : {std::pair{-1, 0}, {1, 1}, {0, -1}, {0, 2}})
    {
        try
        {
            sirkit::zero_one_bfs(1, 0, [&](int, const auto &visit)
                                 {
                                     visit(to, w);
                                 });
            assert(false);
        }
        catch (const std::out_of_range &)
        {
            assert(to != 0);
        }
        catch (const std::invalid_argument &)
        {
            assert(to == 0);
        }
    }
#endif
}
