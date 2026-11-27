// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Three_Edge_Connected_Components.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

void intersect_components(std::vector<std::vector<bool>> &same, const std::vector<Edge> &edges,
                          std::size_t first, std::size_t second)
{
    const auto n = same.size();
    std::vector<std::vector<int>> graph(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        if (id != first && id != second)
        {
            const auto [u, v] = edges[id];
            graph[u].push_back(v);
            graph[v].push_back(u);
        }
    }
    std::vector<int> component(n, -1);
    for (int root = 0; root < static_cast<int>(n); ++root)
    {
        if (component[root] < 0)
        {
            component[root] = root;
            std::vector<int> queue{root};
            for (std::size_t i = 0; i < queue.size(); ++i)
            {
                for (int v : graph[queue[i]])
                {
                    if (component[v] < 0)
                    {
                        component[v] = root;
                        queue.push_back(v);
                    }
                }
            }
        }
    }
    for (std::size_t u = 0; u < n; ++u)
    {
        for (std::size_t v = 0; v < n; ++v)
        {
            same[u][v] = same[u][v] && component[u] == component[v];
        }
    }
}

void check(int n, const std::vector<Edge> &edges)
{
    // 削除する0～2辺の選び方をすべて試し、各場合の連結性をBFSで求める。
    std::vector<std::vector<bool>> expected(n, std::vector<bool>(n, true));
    const auto m = edges.size();
    intersect_components(expected, edges, m, m);
    for (std::size_t i = 0; i < m; ++i)
    {
        for (std::size_t j = i + 1; j <= m; ++j)
        {
            intersect_components(expected, edges, i, j);
        }
    }

    const auto result = sirkit::three_edge_connected_component_ids(n, edges);
    const auto groups = sirkit::three_edge_connected_components(n, edges);
    assert(result.ids.size() == static_cast<std::size_t>(n));
    assert(groups.size() == static_cast<std::size_t>(result.count));
    std::vector<bool> seen(n);
    for (int id = 0; id < result.count; ++id)
    {
        assert(!groups[id].empty());
        for (int v : groups[id])
        {
            assert(0 <= v && v < n);
            assert(!seen[v] && result.ids[v] == id);
            seen[v] = true;
        }
    }
    for (int u = 0; u < n; ++u)
    {
        assert(seen[u]);
        for (int v = 0; v < n; ++v)
        {
            assert((result.ids[u] == result.ids[v]) == expected[u][v]);
        }
    }
}
} // namespace

int main()
{
    using sirkit::three_edge_connected_component_ids;
    check(0, {});
    check(3, {{0, 0}, {1, 1}});
    check(2, {{0, 1}, {1, 0}});
    check(2, {{0, 1}, {1, 0}, {0, 1}});
    check(5, {{0, 2}, {2, 1}, {0, 3}, {3, 1}, {0, 4}, {4, 1}}); // 各枝を切れても両端は3辺連結。
    check(5, {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {0, 2}, {1, 3}, {2, 4}});

    for (int n = 1; n <= 5; ++n)
    {
        std::vector<Edge> all;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u + 1; v < n; ++v)
            {
                all.emplace_back(u, v);
            }
        }
        for (unsigned mask = 0; mask < (1U << all.size()); ++mask)
        {
            std::vector<Edge> edges;
            for (auto left = mask; left != 0; left &= left - 1)
            {
                edges.push_back(all[std::countr_zero(left)]);
            }
            check(n, edges);
        }
    }
    std::mt19937 random(95161);
    for (int trial = 0; trial < 620; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 9);
        const unsigned m = random() % 19;
        std::vector<Edge> edges;
        for (unsigned i = 0; i < m; ++i)
        {
            const int u = static_cast<int>(random() % n);
            const int v = static_cast<int>(random() % n);
            edges.emplace_back(u, v);
        }
        check(n, edges);
        std::reverse(edges.begin(), edges.end());
        if (trial < 40)
        {
            check(n, edges);
        }
    }

    constexpr int kVertices = 10'000;
    std::vector<Edge> chain;
    for (int v = 1; v < kVertices; ++v)
    {
        chain.emplace_back(v - 1, v);
    }
    assert(three_edge_connected_component_ids(kVertices, chain).count == kVertices);
    chain.emplace_back(kVertices - 1, 0);
    assert(three_edge_connected_component_ids(kVertices, chain).count == kVertices);

    try
    {
        static_cast<void>(three_edge_connected_component_ids(-1, {}));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const Edge &invalid : std::array{Edge{-1, 0}, Edge{1, 0}, Edge{0, -1}, Edge{0, 1}})
    {
        try
        {
            static_cast<void>(three_edge_connected_component_ids(1, {&invalid, 1}));
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
