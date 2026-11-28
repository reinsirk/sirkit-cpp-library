// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Undirected_Pseudoforest.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <queue>
#include <random>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
using Graph = sirkit::UndirectedPseudoforest;
using Edge = std::pair<int, int>;
using Adjacent = std::vector<std::vector<std::pair<int, int>>>;

std::vector<int> bfs(const Adjacent &graph, int start, int removed = -1)
{
    std::vector<int> distance(graph.size(), -1);
    std::queue<int> pending;
    pending.push(start);
    distance[start] = 0;
    while (!pending.empty())
    {
        const int u = pending.front();
        pending.pop();
        for (const auto &[v, id] : graph[u])
        {
            if (id != removed && distance[v] == -1)
            {
                distance[v] = distance[u] + 1;
                pending.push(v);
            }
        }
    }
    return distance;
}

int path_count(const Adjacent &graph, int start, int target, unsigned seen)
{
    if (start == target)
    {
        return 1;
    }
    int count = 0;
    for (const auto &[v, id] : graph[start])
    {
        static_cast<void>(id);
        if (!(seen & (1U << v)))
        {
            count += path_count(graph, v, target, seen | (1U << v));
        }
    }
    return count;
}

void check(int n, const std::vector<Edge> &edges)
{
    Adjacent adjacent(n);
    for (int id = 0; id < static_cast<int>(edges.size()); ++id)
    {
        const auto &[u, v] = edges[id];
        adjacent[u].emplace_back(v, id);
        adjacent[v].emplace_back(u, id);
    }
    int components = 0;
    int cycles = 0;
    std::vector<bool> seen(n);
    bool valid = true;
    for (int u = 0; u < n; ++u)
    {
        if (seen[u])
        {
            continue;
        }
        ++components;
        const auto distance = bfs(adjacent, u);
        std::size_t vertices = 0;
        std::size_t arcs = 0;
        for (int v = 0; v < n; ++v)
        {
            if (distance[v] >= 0)
            {
                seen[v] = true;
                ++vertices;
                arcs += adjacent[v].size();
            }
        }
        valid = valid && arcs / 2 <= vertices;
        cycles += arcs / 2 == vertices;
    }

    const auto graph = Graph::build(n, edges);
    assert(graph.has_value() == valid);
    if (!graph)
    {
        return;
    }
    assert(graph->size() == n && graph->component_count() == components && graph->cycle_count() == cycles);
    std::vector<bool> cyclic_edge(edges.size());
    for (int id = 0; id < cycles; ++id)
    {
        const auto cycle = graph->cycle(id);
        assert(!cycle.vertices.empty());
        assert(cycle.vertices.size() == cycle.edge_ids.size());
        for (std::size_t i = 0; i < cycle.vertices.size(); ++i)
        {
            const int from = cycle.vertices[i];
            const int to = cycle.vertices[(i + 1) % cycle.vertices.size()];
            const int edge = cycle.edge_ids[i];
            assert(!cyclic_edge[edge]);
            cyclic_edge[edge] = true;
            assert((edges[edge] == Edge{from, to} || edges[edge] == Edge{to, from}));
            assert(graph->root(from) == from && graph->cycle_id(from) == id);
        }
    }
    for (int id = 0; id < static_cast<int>(edges.size()); ++id)
    {
        const auto [u, v] = edges[id];
        assert(cyclic_edge[id] == (bfs(adjacent, u, id)[v] != -1));
    }
    for (int u = 0; u < n; ++u)
    {
        const auto reference = bfs(adjacent, u);
        const int leader = graph->root(u);
        assert(graph->root(leader) == leader && reference[leader] == graph->depth(u));
        for (int v = 0; v < n; ++v)
        {
            assert(graph->distance(u, v) == reference[v]);
            assert(graph->unique_path(u, v) == (path_count(adjacent, u, v, 1U << u) == 1));
            assert((graph->component(u) == graph->component(v)) == (reference[v] != -1));
        }
    }
}

template <class Exception, class Operation>
void must_throw(Operation invoke)
{
    bool caught = false;
    try
    {
        invoke();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

void check_boundaries()
{
    check(0, {});
    check(1, {});
    check(1, {{0, 0}});
    check(3, {{0, 0}, {1, 2}, {1, 2}});
    check(6, {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {3, 4}, {3, 5}});
    check(7, {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {3, 4}, {4, 0}});
    check(10, {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {2, 5}, {2, 6}, {3, 7}, {5, 8}, {5, 9}});
    assert(Graph::build(1, {}).value().cycle_id(0) == -1);
    must_throw<std::length_error>([]
                                  {
                                      static_cast<void>(Graph::build(-1, {}));
                                  });
#ifndef SKIP_RUNTIME_CHECKS
    const auto graph = Graph::build(1, {}).value();
    for (const int v : {-1, 1})
    {
        const std::array bad{Edge{0, v}};
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(Graph::build(1, bad));
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(graph.cycle(v));
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(graph.root(v));
                                      });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(266);
    for (int trial = 0; trial < 1250; ++trial)
    {
        const int n = 1 + static_cast<int>(rng() % 9);
        const int m = static_cast<int>(rng() % (n + 5));
        std::vector<Edge> edges;
        for (int id = 0; id < m; ++id)
        {
            edges.emplace_back(rng() % n, rng() % n);
        }
        check(n, edges);
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        static_cast<void>(Graph::build(-1, {}));
        return 1;
    }
    check_boundaries();
    check_random();
}
