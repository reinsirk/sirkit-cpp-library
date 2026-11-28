// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Subtree_Edge_Add_Distances.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
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

using Tree = sirkit::SubtreeEdgeAddDistances<>;
using Edge = Tree::Edge;

struct Reference
{
    std::vector<std::vector<std::pair<int, int>>> adjacency;
    std::vector<Edge> edges;
    std::vector<int> parent;
    std::vector<int> root;

    Reference(int n, std::vector<Edge> values, int first_root)
        : adjacency(n),
          edges(std::move(values)),
          parent(n, -1),
          root(n, -1)
    {
        for (int id = 0; id < static_cast<int>(edges.size()); ++id)
        {
            const auto [u, v, w] = edges[id];
            adjacency[u].emplace_back(v, id);
            adjacency[v].emplace_back(u, id);
        }
        auto visit = [&](auto &self, int v, int component_root) -> void
        {
            root[v] = component_root;
            for (const auto &[next, id] : adjacency[v])
            {
                if (next != parent[v])
                {
                    parent[next] = v;
                    self(self, next, component_root);
                }
            }
        };
        visit(visit, first_root, first_root);
        for (int v = 0; v < n; ++v)
        {
            if (root[v] == -1)
            {
                visit(visit, v, v);
            }
        }
    }

    void update(int v, std::int64_t delta)
    {
        for (const auto &[next, id] : adjacency[v])
        {
            if (next != parent[v])
            {
                edges[id].weight += delta;
                update(next, delta);
            }
        }
    }

    std::optional<std::int64_t> distance(int from, int target) const
    {
        auto visit = [&](auto &self, int v, int previous, std::int64_t sum) -> std::optional<std::int64_t>
        {
            if (v == target)
            {
                return sum;
            }
            for (const auto &[next, id] : adjacency[v])
            {
                if (next != previous)
                {
                    if (auto result = self(self, next, v, sum + edges[id].weight))
                    {
                        return result;
                    }
                }
            }
            return std::nullopt;
        };
        return visit(visit, from, -1, 0);
    }
};
} // namespace

int main()
{
    Tree empty(0, {});
    assert(empty.size() == 0);

    const std::vector<Edge> small{{0, 1, 8}, {2, 1, -3}, {2, 3, 9}, {4, 5, 7}};
    Tree test(7, small, 2);
    assert(test.size() == 7 && test.distance(0, 5) == std::nullopt);
    assert(test.root_distance(0) == 5);
    assert(test.root_distance(3) == 9);
    assert(test.root_distance(5) == 7);
    test.add_subtree(1, -4);
    assert(test.root_distance(0) == 1);
    assert(test.root_distance(3) == 9);
    assert(test.root_distance(5) == 7);
    assert(test.distance(0, 3) == 10);
    assert(test.distance(1, 0) == 4);
    test.add_subtree(0, 18);
    assert(test.distance(1, 0) == 4);
    auto copy = test;
    copy.add_subtree(2, 7);
    assert(copy.distance(0, 3) == 31 && test.distance(0, 3) == 10);
    copy.add_subtree(6, 99);
    assert(copy.root_distance(6) == 0);

    using Short = sirkit::SubtreeEdgeAddDistances<std::int8_t>;
    const std::vector<Short::Edge> short_edges{{0, 1, 120}, {1, 2, 120}};
    Short short_tree(3, short_edges);
    assert(short_tree.root_distance(2) == -16);
    short_tree.add_subtree(1, 10);
    assert(short_tree.distance(2, 1) == -126);
    assert(short_tree.root_distance(2) == -6);
    const std::vector<Edge> large{{0, 1, std::numeric_limits<std::int64_t>::max()}, {1, 2, 3}};
    Tree wide(3, large);
    wide.add_subtree(1, -4);
    assert(wide.distance(0, 2) == std::numeric_limits<std::int64_t>::max() - 1);
    wide.add_subtree(0, std::numeric_limits<std::int64_t>::min());
    assert(wide.distance(0, 2) == std::numeric_limits<std::int64_t>::max() - 1);

    std::mt19937 random(5600890);
    for (int test_id = 0; test_id < 600; ++test_id)
    {
        const int n = static_cast<int>(random() % 20) + 1;
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            if (random() % 4 != 0)
            {
                const int parent = static_cast<int>(random() % v);
                const int weight = static_cast<int>(random() % 51) - 25;
                edges.push_back(random() % 2 ? Edge{parent, v, weight} : Edge{v, parent, weight});
            }
        }
        std::shuffle(edges.begin(), edges.end(), random);
        const int root = static_cast<int>(random() % n);
        Tree tree(n, edges, root);
        Reference oracle(n, edges, root);
        for (int i = 0; i < 90; ++i)
        {
            const int u = static_cast<int>(random() % n);
            const int v = static_cast<int>(random() % n);
            if (random() % 2)
            {
                const int delta = static_cast<int>(random() % 61) - 30;
                tree.add_subtree(u, delta);
                oracle.update(u, delta);
            }
            assert(tree.root_distance(u) == oracle.distance(oracle.root[u], u));
            assert(tree.distance(u, v) == oracle.distance(u, v));
        }
    }

    check_error<std::length_error>([] { Tree invalid(-1, {}); });
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::out_of_range>([] { Tree invalid(2, {}, 2); });
    const std::vector<Edge> invalid_edge{{0, 2, 3}};
    check_error<std::out_of_range>([&] { Tree invalid(2, invalid_edge); });
    const std::vector<Edge> cycle{{0, 1, 1}, {1, 2, 2}, {0, 2, 3}};
    check_error<std::invalid_argument>([&] { Tree invalid(3, cycle); });
    check_error<std::out_of_range>([&] { test.add_subtree(-1, 4); });
    check_error<std::out_of_range>([&] { (void)test.root_distance(7); });
    check_error<std::out_of_range>([&] { (void)test.distance(0, 7); });
#endif
}
