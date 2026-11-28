// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Path_Difference.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
template <typename Error, typename Action>
void check_error(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}

using Tree = sirkit::TreePathDifference<>;
using Edge = std::pair<int, int>;
} // namespace

int main()
{
    const Tree empty(0, {});
    assert(empty.vertex_values().empty() && empty.edge_values().empty());

    const std::vector<Edge> small{{0, 1}, {2, 1}, {4, 3}};
    Tree paths(6, small, 2);
    assert(paths.add_vertex_path(0, 2, 7));
    assert(paths.add_vertex_path(1, 1, -9));
    assert(paths.add_vertex_path(2, 2, 1));
    assert(!paths.add_vertex_path(0, 5, 100));
    assert(paths.add_edge_path(1, 0, -3));
    assert(paths.add_edge_path(1, 2, -8));
    assert(paths.add_edge_path(1, 1, 25));
    assert(!paths.add_edge_path(3, 2, -100));
    assert((paths.vertex_values() == std::vector<std::int64_t>{7, -2, 8, 0, 0, 0}));
    assert((paths.edge_values() == std::vector<std::int64_t>{-3, -8, 0}));
    auto copy = paths;
    assert(copy.add_vertex_path(3, 4, 5));
    assert(copy.add_edge_path(3, 4, 11));
    assert((copy.vertex_values() == std::vector<std::int64_t>{7, -2, 8, 5, 5, 0}));
    assert((copy.edge_values() == std::vector<std::int64_t>{-3, -8, 11}));
    assert((paths.edge_values() == std::vector<std::int64_t>{-3, -8, 0}));

    sirkit::TreePathDifference<std::int8_t> low(6, small);
    assert(low.add_edge_path(0, 2, -120));
    assert(low.add_edge_path(0, 1, -10));
    assert(low.add_vertex_path(0, 2, 120));
    assert(low.add_vertex_path(0, 0, 10));
    assert((low.edge_values() == std::vector<std::int8_t>{126, -120, 0}));
    assert((low.vertex_values() == std::vector<std::int8_t>{-126, 120, 120, 0, 0, 0}));
    Tree large(6, small);
    constexpr auto kMinimum = std::numeric_limits<std::int64_t>::min();
    assert(large.add_vertex_path(0, 1, kMinimum));
    assert(large.add_edge_path(0, 1, kMinimum));
    assert((large.vertex_values() == std::vector<std::int64_t>{kMinimum, kMinimum, 0, 0, 0, 0}));
    assert((large.edge_values() == std::vector<std::int64_t>{kMinimum, 0, 0}));

    std::mt19937 random(732993);
    for (int test = 0; test < 775; ++test)
    {
        const int n = static_cast<int>(random() % 20) + 1;
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            if (random() % 4 != 0)
            {
                const int u = static_cast<int>(random() % v);
                edges.push_back(random() % 2 != 0 ? Edge{u, v} : Edge{v, u});
            }
        }
        std::shuffle(edges.begin(), edges.end(), random);
        std::vector<std::vector<Edge>> graph(n);
        for (int id = 0; id < static_cast<int>(edges.size()); ++id)
        {
            const auto [u, v] = edges[id];
            graph[u].emplace_back(v, id);
            graph[v].emplace_back(u, id);
        }
        Tree difference(n, edges, static_cast<int>(random() % n));
        std::vector<std::int64_t> vertices(n), weights(edges.size());
        for (int query = 0; query < 55; ++query)
        {
            const int source = static_cast<int>(random() % n);
            const int target = static_cast<int>(random() % n);
            const int delta = static_cast<int>(random() % 99) - 49;
            const bool vertex_values = random() % 2 == 0;
            auto visit = [&](auto &self, int v, int parent) -> bool
            {
                if (v == target)
                {
                    if (vertex_values)
                    {
                        vertices[v] += delta;
                    }
                    return true;
                }
                for (const auto &[next, id] : graph[v])
                {
                    if (next != parent && self(self, next, v))
                    {
                        if (vertex_values)
                        {
                            vertices[v] += delta;
                        }
                        else
                        {
                            weights[id] += delta;
                        }
                        return true;
                    }
                }
                return false;
            };
            const bool reachable = visit(visit, source, -1);
            const bool success = vertex_values ? difference.add_vertex_path(source, target, delta)
                                               : difference.add_edge_path(source, target, delta);
            assert(success == reachable);
            if (query % 17 == 0)
            {
                assert(difference.vertex_values() == vertices);
                assert(difference.edge_values() == weights);
            }
        }
        assert(difference.vertex_values() == vertices);
        assert(difference.edge_values() == weights);
    }

    check_error<std::length_error>([] { Tree bad(-1, {}); });
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::out_of_range>([] { Tree bad(2, {}, -1); });
    const std::vector<Edge> invalid{{2, 0}};
    check_error<std::out_of_range>([&] { Tree bad(2, invalid); });
    const std::vector<Edge> cycle{{0, 1}, {1, 2}, {2, 0}};
    check_error<std::invalid_argument>([&] { Tree bad(3, cycle); });
    check_error<std::out_of_range>([&] { paths.add_vertex_path(-1, 0, 1); });
    check_error<std::out_of_range>([&] { paths.add_edge_path(0, 6, 1); });
#endif
}
