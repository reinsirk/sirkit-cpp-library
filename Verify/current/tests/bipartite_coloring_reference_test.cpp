// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Bipartite_Coloring.cpp"

#include <array>
#include <cassert>
#include <random>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

bool brute(int n, const std::vector<Edge> &edges)
{
    for (unsigned mask = 0; mask < (1U << n); ++mask)
    {
        bool works = true;
        for (const auto &[u, v] : edges)
        {
            works = works && (((mask >> u) ^ (mask >> v)) & 1U) != 0;
        }
        if (works)
        {
            return true;
        }
    }
    return false;
}

void check(int n, const std::vector<Edge> &edges)
{
    const auto result = sirkit::bipartite_coloring(n, edges);
    assert(result.is_bipartite() == brute(n, edges));
    if (result.is_bipartite())
    {
        assert(result.color.size() == static_cast<std::size_t>(n));
        for (const int c : result.color)
        {
            assert(c == 0 || c == 1);
        }
        for (const auto &[u, v] : edges)
        {
            assert(result.color[u] != result.color[v]);
        }
    }
    else
    {
        assert(result.color.empty());
        const auto &cycle = result.odd_cycle.value();
        const auto length = cycle.vertices.size();
        assert(length % 2 == 1 && length == cycle.edge_ids.size());
        std::vector<bool> used_vertex(n);
        std::vector<bool> used_edge(edges.size());
        for (std::size_t i = 0; i < length; ++i)
        {
            const auto id = cycle.edge_ids[i];
            assert(id < edges.size() && !used_edge[id]);
            used_edge[id] = true;
            const int a = cycle.vertices[i];
            const int b = cycle.vertices[(i + 1) % length];
            assert(0 <= a && a < n && !used_vertex[a]);
            used_vertex[a] = true;
            assert((edges[id] == Edge{a, b} || edges[id] == Edge{b, a}));
        }
    }
}

template <class Exception, class Operation>
void must_throw(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

void check_limits()
{
    const std::array triangle{Edge{0, 1}, Edge{1, 2}, Edge{2, 0}};
    assert(sirkit::find_cycle(3, triangle).value().vertices.size() == 3);
    check(0, {});
    check(1, {{0, 0}});
    check(5, {{0, 1}, {0, 1}, {2, 3}});
    check(6, {{0, 1}, {1, 2}, {0, 3}, {3, 4}, {2, 4}});
    must_throw<std::length_error>([]
                                  {
                                      static_cast<void>(sirkit::bipartite_coloring(-1, {}));
                                  });
#ifndef SKIP_RUNTIME_CHECKS
    for (const Edge &bad : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(sirkit::bipartite_coloring(2, {&bad, 1}));
                                      });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(327);
    for (int trial = 0; trial < 1500; ++trial)
    {
        const int n = 1 + static_cast<int>(rng() % 10);
        const unsigned m = rng() % 29;
        const bool require_bipartite = trial % 2 == 0;
        std::vector<Edge> edges;
        for (unsigned i = 0; i < m; ++i)
        {
            const int u = static_cast<int>(rng() % n);
            const int v = static_cast<int>(rng() % n);
            if (!require_bipartite || (u % 2 != v % 2))
            {
                edges.emplace_back(u, v);
            }
        }
        check(n, edges);
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        static_cast<void>(sirkit::bipartite_coloring(-1, {}));
        return 1;
    }
    check_limits();
    check_random();
}
