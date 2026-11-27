// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_DYNAMIC_BIPARTITENESS_DEFINED
#define SIRKIT_DYNAMIC_BIPARTITENESS_REFERENCE_SOURCE "../../../Graph/Dynamic_Bipartiteness.cpp"
#include SIRKIT_DYNAMIC_BIPARTITENESS_REFERENCE_SOURCE
#undef SIRKIT_DYNAMIC_BIPARTITENESS_REFERENCE_SOURCE
#endif

#include <array>
#include <cassert>
#include <cstddef>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
struct Edge
{
    int u, v;
    bool active;
};

bool IsBipartite(int n, const std::vector<Edge> &edges)
{
    std::vector<std::vector<int>> adjacency(n);
    for (const auto &edge : edges)
    {
        if (edge.active)
        {
            adjacency[edge.u].push_back(edge.v);
            adjacency[edge.v].push_back(edge.u);
        }
    }

    std::vector<int> color(n, -1);
    std::queue<int> queue;
    for (int start = 0; start < n; ++start)
    {
        if (color[start] != -1)
        {
            continue;
        }
        queue.push(start);
        color[start] = 0;
        while (!queue.empty())
        {
            const auto u = queue.front();
            queue.pop();
            for (const auto v : adjacency[u])
            {
                if (color[v] == color[u])
                {
                    return false;
                }
                if (color[v] == -1)
                {
                    color[v] = color[u] ^ 1;
                    queue.push(v);
                }
            }
        }
    }
    return true;
}
}

int main()
{
    using sirkit::DynamicBipartiteness;
    DynamicBipartiteness empty;
    assert(empty.size() == 0 && empty.edge_count() == 0 && empty.query_count() == 0);
    assert(empty.solve().empty());
    assert(empty.query_bipartite() == 0);
    assert(empty.solve() == std::vector<bool>{true});
    DynamicBipartiteness unobserved(1);
    const auto before = unobserved.add_edge(0, 0);
    unobserved.remove_edge(before);
    unobserved.query_bipartite();
    unobserved.add_edge(0, 0);
    assert(unobserved.solve() == std::vector<bool>{true});
    unobserved.query_bipartite();
    assert((unobserved.solve() == std::vector<bool>{true, false}));

    DynamicBipartiteness graph(5);
    const auto first = graph.add_edge(0, 1);
    const auto parallel = graph.add_edge(1, 0);
    graph.add_edge(1, 2);
    graph.add_edge(3, 4);
    graph.query_bipartite();
    const auto triangle = graph.add_edge(0, 2);
    graph.query_bipartite();
    graph.remove_edge(first);
    graph.query_bipartite();
    graph.remove_edge(parallel);
    graph.query_bipartite();
    assert((graph.solve() == std::vector<bool>{true, false, false, true}));

    const auto loop = graph.add_edge(3, 3);
    graph.query_bipartite();
    graph.remove_edge(triangle);
    graph.query_bipartite();
    graph.remove_edge(loop);
    graph.query_bipartite();
    graph.query_bipartite();
    assert(graph.size() == 5 && graph.edge_count() == 6 && graph.query_count() == 8);
    assert((graph.solve() == std::vector<bool>{true, false, false, true, false, false, true, true}));

    std::mt19937 rng(9271237);
    for (int trial = 0; trial < 1200; ++trial)
    {
        const int n = trial % 15 + 1;
        DynamicBipartiteness candidate(n);
        std::vector<Edge> edges;
        std::vector<std::size_t> active;
        std::vector<bool> expected;
        for (int step = 0; step < 90; ++step)
        {
            const auto type = rng() % 7;
            if (type <= 2)
            {
                const int u = static_cast<int>(rng() % n);
                const int v = static_cast<int>(rng() % n);
                assert(candidate.add_edge(u, v) == edges.size());
                active.push_back(edges.size());
                edges.push_back({u, v, true});
            }
            else if (type <= 4 && !active.empty())
            {
                const auto index = rng() % active.size();
                const auto id = active[index];
                candidate.remove_edge(id);
                edges[id].active = false;
                active[index] = active.back();
                active.pop_back();
            }
            else
            {
                assert(candidate.query_bipartite() == expected.size());
                expected.push_back(IsBipartite(n, edges));
            }
            if (step % 11 == 0)
            {
                assert(candidate.solve() == expected);
            }
        }
        assert(candidate.solve() == expected);
    }

    sirkit::detail::CheckDynamicBipartitenessLimit(3, 3);
    try
    {
        sirkit::detail::CheckDynamicBipartitenessLimit(4, 3);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }

#ifndef SKIP_RUNTIME_CHECKS
    try
    {
        DynamicBipartiteness bad(-1);
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    constexpr std::array bad_edges{std::pair{-1, 1}, std::pair{5, 1}, std::pair{1, -1}, std::pair{1, 5}};
    for (const auto &[u, v] : bad_edges)
    {
        try
        {
            graph.add_edge(u, v);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
    try
    {
        graph.remove_edge(graph.edge_count());
        assert(false);
    }
    catch (const std::out_of_range &)
    {
    }
    try
    {
        graph.remove_edge(first);
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
