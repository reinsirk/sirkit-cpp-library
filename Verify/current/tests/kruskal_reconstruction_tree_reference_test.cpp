// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Kruskal_Reconstruction_Tree.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace
{
using Edge = sirkit::SpanningTreeEdge;
using Weight = std::int64_t;

void check(int n, const std::vector<Edge> &edges, bool increasing)
{
    using sirkit::KruskalOrder;
    const sirkit::KruskalReconstructionTree tree(n, edges, increasing ? KruskalOrder::increasing : KruskalOrder::decreasing);
    assert(tree.size() == n);
    int seen = 0;
    for (int i = 0; i < tree.node_count(); ++i)
    {
        const auto &node = tree.node(i);
        if (node.parent == -1)
        {
            seen += node.vertex_count;
        }
        else
        {
            assert(i < node.parent);
            const auto &parent = tree.node(node.parent);
            assert(parent.left == i || parent.right == i);
        }
        if (i < n)
        {
            assert(node.left == -1 && node.right == -1 && !node.weight && node.vertex_count == 1);
        }
        else
        {
            assert(node.left < i && node.right < i && node.weight && node.left != node.right);
            assert(node.vertex_count == tree.node(node.left).vertex_count + tree.node(node.right).vertex_count);
        }
    }
    assert(seen == n);

    const auto better = [increasing](Weight a, Weight b)
    {
        return increasing ? a < b : a > b;
    };
    const auto worst = [increasing](Weight a, Weight b)
    {
        return increasing ? std::max(a, b) : std::min(a, b);
    };
    const auto within = [increasing](Weight a, Weight threshold)
    {
        return increasing ? a <= threshold : a >= threshold;
    };
    std::vector<std::vector<std::optional<Weight>>> best(n, std::vector<std::optional<Weight>>(n));
    for (int v = 0; v < n; ++v)
    {
        best[v][v] = increasing ? std::numeric_limits<Weight>::min() : std::numeric_limits<Weight>::max();
    }
    for (const auto &[u, v, weight] : edges)
    {
        if (!best[u][v] || better(weight, *best[u][v]))
        {
            best[u][v] = best[v][u] = weight;
        }
    }
    for (int k = 0; k < n; ++k)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                if (best[u][k] && best[k][v])
                {
                    const auto candidate = worst(*best[u][k], *best[k][v]);
                    if (!best[u][v] || better(candidate, *best[u][v]))
                    {
                        best[u][v] = candidate;
                    }
                }
            }
        }
    }

    const std::array thresholds{std::numeric_limits<Weight>::min(), Weight{-100}, Weight{-5}, Weight{0},
                                Weight{3}, Weight{7}, Weight{100}, std::numeric_limits<Weight>::max()};
    for (int u = 0; u < n; ++u)
    {
        assert(tree.merge_node(u, u) == u && !tree.merge_weight(u, u));
        for (int v = 0; v < n; ++v)
        {
            if (u != v)
            {
                assert(tree.merge_weight(u, v) == best[u][v]);
                assert((tree.merge_node(u, v) != -1) == best[u][v].has_value());
            }
        }
        for (const auto value : thresholds)
        {
            const int id = tree.component_at(u, value);
            int total = 0;
            for (int v = 0; v < n; ++v)
            {
                const bool shares = best[u][v] && within(*best[u][v], value);
                assert(shares == (tree.component_at(v, value) == id));
                total += shares;
            }
            assert(tree.node(id).vertex_count == total);
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

void check_limits()
{
    using sirkit::KruskalReconstructionTree;
    constexpr auto kLowest = std::numeric_limits<Weight>::min();
    constexpr auto kHighest = std::numeric_limits<Weight>::max();
    check(0, {}, true);
    check(1, {{0, 0, kHighest}}, false);
    check(4, {{0, 1, kLowest}, {1, 2, kHighest}, {0, 2, 0}, {0, 0, -10}}, true);
    check(4, {{0, 1, kLowest}, {1, 2, kHighest}, {0, 2, 0}, {0, 0, -10}}, false);
    const std::array edges{Edge{0, 1, 5}, Edge{0, 2, 5}, Edge{1, 2, 5}};
    const KruskalReconstructionTree tied(3, edges);
    assert(tied.merge_node(0, 1) == 3 && tied.merge_node(0, 2) == 4);
    must_throw<std::length_error>([]
                                  {
                                      static_cast<void>(KruskalReconstructionTree(-1, {}));
                                  });
    must_throw<std::length_error>([]
                                  {
                                      static_cast<void>(KruskalReconstructionTree(std::numeric_limits<int>::max() / 2 + 1, {}));
                                  });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int invalid : {-1, 5})
    {
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(tied.node(invalid));
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(tied.merge_node(invalid, 0));
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(tied.component_at(invalid, 5));
                                      });
        const std::array bad{Edge{0, invalid, 0}};
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(KruskalReconstructionTree(2, bad));
                                      });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(394);
    for (int trial = 0; trial < 920; ++trial)
    {
        const int n = 1 + static_cast<int>(rng() % 13);
        const int m = static_cast<int>(rng() % 37);
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            const int u = static_cast<int>(rng() % n);
            const int v = static_cast<int>(rng() % n);
            edges.push_back({u, v, static_cast<int>(rng() % 41) - 20});
        }
        check(n, edges, true);
        check(n, edges, false);
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        static_cast<void>(sirkit::KruskalReconstructionTree(-1, {}));
        return 1;
    }
    check_limits();
    check_random();
}
