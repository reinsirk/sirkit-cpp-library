// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Minimum_Directed_Spanning_Tree.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Edge = sirkit::DirectedMstEdge;
constexpr auto kNoEdge = sirkit::DirectedMstResult::kNoEdge;

bool reaches_root(int n, int root, const std::vector<int> &parent)
{
    for (int v = 0; v < n; ++v)
    {
        int at = v;
        for (int step = 0; step < n && at != root; ++step)
        {
            at = parent[at];
        }
        if (at != root)
        {
            return false;
        }
    }
    return true;
}

std::optional<std::int64_t> brute_force(int n, int root, const std::vector<Edge> &edges)
{
    std::vector<std::vector<std::size_t>> incoming(n);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &[from, to, weight] = edges[i];
        if (to != root && from != to)
        {
            incoming[to].push_back(i);
        }
    }
    std::optional<std::int64_t> best;
    std::vector<int> parent(n, root);
    const auto search = [&](auto &&self, int v, std::int64_t cost) -> void
    {
        if (v == n)
        {
            if (reaches_root(n, root, parent) && (!best || cost < *best))
            {
                best = cost;
            }
        }
        else if (v == root)
        {
            self(self, v + 1, cost);
        }
        else
        {
            for (auto edge : incoming[v])
            {
                parent[v] = edges[edge].from;
                self(self, v + 1, cost + edges[edge].weight);
            }
        }
    };
    search(search, 0, 0);
    return best;
}

void check_small(int n, int root, const std::vector<Edge> &edges)
{
    const auto actual = sirkit::minimum_directed_spanning_tree(n, root, edges);
    const auto expected = brute_force(n, root, edges);
    assert(actual.has_value() == expected.has_value());
    if (!actual)
    {
        return;
    }
    assert(actual->cost == *expected);
    assert(actual->incoming_edge.size() == static_cast<std::size_t>(n));
    assert(actual->incoming_edge[root] == kNoEdge);

    std::vector<int> parent(n, root);
    std::int64_t total = 0;
    for (int v = 0; v < n; ++v)
    {
        if (v != root)
        {
            const auto id = actual->incoming_edge[v];
            assert(id < edges.size());
            const auto &[from, to, weight] = edges[id];
            assert(to == v && from != v);
            parent[v] = from;
            total += weight;
        }
    }
    assert(total == actual->cost);
    assert(reaches_root(n, root, parent));
}

template <class Exception>
void expect_error(int n, int root, std::initializer_list<Edge> edges)
{
    try
    {
        static_cast<void>(sirkit::minimum_directed_spanning_tree(n, root, {edges.begin(), edges.size()}));
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
} // namespace

int main()
{
    check_small(1, 0, {});
    check_small(3, 1, {});
    check_small(3, 0, {{1, 2, 0}, {2, 1, 0}});
    check_small(3, 2, {{2, 1, 4}, {2, 0, 3}, {0, 1, 1}, {0, 1, 2}, {1, 1, -9}, {0, 2, -9}});
    check_small(5, 4, {{4, 0, 5}, {0, 1, -2}, {1, 0, -2}, {4, 2, 5}, {2, 3, -3}, {3, 2, -3}});
    check_small(5, 0, {{2, 1, 0}, {1, 2, 0}, {3, 1, 1}, {1, 3, 0}, {4, 1, 2}, {1, 4, 0}, {0, 1, 3}});
    std::mt19937 random(122023);
    for (int n = 1; n <= 7; ++n)
    {
        for (int trial = 0; trial < 2500; ++trial)
        {
            const int root = static_cast<int>(random() % n);
            const auto density = random() % 101;
            std::vector<Edge> edges;
            for (int u = 0; u < n; ++u)
            {
                for (int v = 0; v < n; ++v)
                {
                    if (random() % 100 < density)
                    {
                        edges.push_back({u, v, static_cast<std::int64_t>(random() % 25) - 12});
                        if (random() % 16 == 0)
                        {
                            edges.push_back({u, v, static_cast<std::int64_t>(random() % 25) - 12});
                        }
                    }
                }
            }
            check_small(n, root, edges);
        }
    }

    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    const std::vector<Edge> exact_min{{0, 1, kMin}};
    const std::vector<Edge> exact_max{{0, 1, kMax}};
    assert(sirkit::minimum_directed_spanning_tree(2, 0, exact_min)->cost == kMin);
    assert(sirkit::minimum_directed_spanning_tree(2, 0, exact_max)->cost == kMax);

    const std::vector<Edge> cancel{{0, 1, kMax}, {1, 2, kMax}, {2, 3, kMin}};
    assert(sirkit::minimum_directed_spanning_tree(4, 0, cancel)->cost == kMax - 1);
    const std::vector<Edge> wide_cycle{{1, 2, kMin}, {2, 1, kMin}, {0, 1, kMax}, {0, 2, kMax}};
    const auto extreme = sirkit::minimum_directed_spanning_tree(3, 0, wide_cycle);
    assert(extreme && extreme->cost == -1);

    expect_error<std::overflow_error>(3, 0, {{0, 1, kMax}, {1, 2, kMax}});
    expect_error<std::overflow_error>(3, 0, {{0, 1, kMin}, {1, 2, kMin}});
    expect_error<std::invalid_argument>(0, 0, {});
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::out_of_range>(2, -1, {});
    expect_error<std::out_of_range>(2, 2, {});
    expect_error<std::out_of_range>(2, 0, {{-1, 1, 0}});
    expect_error<std::out_of_range>(2, 0, {{2, 1, 0}});
    expect_error<std::out_of_range>(2, 0, {{0, -1, 0}});
    expect_error<std::out_of_range>(2, 0, {{0, 2, 0}});
#endif

    // 縮約木の深さが頂点数に比例する入力。
    constexpr int kSize = 40000;
    std::vector<Edge> nested{{0, 1, kSize}, {1, 2, 0}, {2, 1, 0}};
    for (int v = 3; v < kSize; ++v)
    {
        nested.push_back({v, 1, v});
        nested.push_back({1, v, 0});
    }
    const auto result = sirkit::minimum_directed_spanning_tree(kSize, 0, nested);
    assert(result && result->cost == kSize);
    assert(result->incoming_edge[0] == kNoEdge && result->incoming_edge[1] == 0);
    for (int v = 2; v < kSize; ++v)
    {
        const auto &edge = nested[result->incoming_edge[v]];
        assert(edge.from == 1 && edge.to == v);
    }
}
