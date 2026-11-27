// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Cycle_Detection.cpp"
#include <cassert>
#include <numeric>
#include <random>
#include <set>
#include <utility>
#include <vector>

int main()
{
    try
    {
        sirkit::detail::cycle_arc_count(4, 2, 7);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    std::mt19937 random(4816);
    for (int n = 0; n <= 12; ++n)
    {
        for (int trial = 0; trial < 80; ++trial)
        {
            std::vector<std::pair<int, int>> edges;
            const auto m = random() % 25;
            for (unsigned i = 0; n != 0 && i < m; ++i)
            {
                edges.emplace_back(random() % n, random() % n);
            }
            for (bool directed : {false, true})
            {
                bool expected = false;
                if (directed)
                {
                    std::vector<std::vector<bool>> reachable(n, std::vector<bool>(n));
                    for (auto [u, v] : edges)
                    {
                        reachable[u][v] = true;
                    }
                    for (int k = 0; k < n; ++k)
                    {
                        for (int i = 0; i < n; ++i)
                        {
                            for (int j = 0; j < n; ++j)
                            {
                                reachable[i][j] = reachable[i][j] || (reachable[i][k] && reachable[k][j]);
                            }
                        }
                    }
                    for (int v = 0; v < n; ++v)
                    {
                        expected = expected || reachable[v][v];
                    }
                }
                else
                {
                    std::vector<int> component(n);
                    std::iota(component.begin(), component.end(), 0);
                    for (auto [u, v] : edges)
                    {
                        if (component[u] == component[v])
                        {
                            expected = true;
                        }
                        const int old = component[v], replacement = component[u];
                        for (int &label : component)
                        {
                            if (label == old)
                            {
                                label = replacement;
                            }
                        }
                    }
                }
                const auto result = sirkit::find_cycle(n, edges, directed);
                assert(bool(result) == expected);
                if (!result)
                {
                    continue;
                }
                const auto &ids = result->edge_ids;
                const auto &vertices = result->vertices;
                assert(!ids.empty() && ids.size() == vertices.size());
                assert(std::set<std::size_t>(ids.begin(), ids.end()).size() == ids.size());
                assert(std::set<int>(vertices.begin(), vertices.end()).size() == vertices.size());
                for (std::size_t i = 0; i < ids.size(); ++i)
                {
                    assert(ids[i] < edges.size());
                    const auto [from, to] = edges[ids[i]];
                    const auto u = vertices[i], v = vertices[(i + 1) % ids.size()];
                    assert((from == u && to == v) || (!directed && from == v && to == u));
                }
            }
        }
    }
    const std::vector<std::pair<int, int>> parallel{{0, 1}, {0, 1}};
    assert(sirkit::find_cycle(2, parallel, false)->edge_ids.size() == 2);
    assert(!sirkit::find_cycle(2, parallel));
    try
    {
        sirkit::find_cycle(-1, {});
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &bad : {std::pair{-1, 0}, {2, 0}, {0, -1}, {0, 2}})
    {
        try
        {
            sirkit::find_cycle(2, {&bad, 1});
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
    try
    {
        sirkit::find_cycle(1, parallel);
        assert(false);
    }
    catch (const std::out_of_range &)
    {
    }
#endif
}
