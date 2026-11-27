// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Chordal_Graph.cpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

bool contains_hole(const std::vector<unsigned> &adjacency)
{
    for (unsigned set = 0; set < (1U << adjacency.size()); ++set)
    {
        if (std::popcount(set) < 4)
        {
            continue;
        }
        bool cycle_degrees = true;
        for (auto left = set; left != 0; left &= left - 1)
        {
            if (std::popcount(adjacency[std::countr_zero(left)] & set) != 2)
            {
                cycle_degrees = false;
                break;
            }
        }
        if (!cycle_degrees)
        {
            continue;
        }
        unsigned reached = set & (0U - set);
        for (unsigned todo = reached; todo != 0;)
        {
            const auto next = adjacency[std::countr_zero(todo)] & set & ~reached;
            reached |= next;
            todo = (todo & (todo - 1)) | next;
        }
        if (reached == set)
        {
            return true;
        }
    }
    return false;
}

void check_small(int n, const std::vector<Edge> &edges)
{
    std::vector<unsigned> adjacency(n);
    for (const auto &[u, v] : edges)
    {
        if (u != v)
        {
            adjacency[u] |= 1U << v;
            adjacency[v] |= 1U << u;
        }
    }
    const auto result = sirkit::recognize_chordal_graph(n, edges);
    assert(result.is_chordal == !contains_hole(adjacency));

    if (result.is_chordal)
    {
        assert(result.induced_cycle.empty());
        assert(result.perfect_elimination_order.size() == static_cast<std::size_t>(n));
        unsigned remaining = (1U << n) - 1;
        for (int v : result.perfect_elimination_order)
        {
            assert(v >= 0 && v < n && (remaining & (1U << v)));
            remaining ^= 1U << v;
            const auto later_neighbors = remaining & adjacency[v];
            for (auto left = later_neighbors; left != 0; left &= left - 1)
            {
                const int u = std::countr_zero(left);
                assert((later_neighbors & ~(1U << u) & ~adjacency[u]) == 0);
            }
        }
    }
    else
    {
        assert(result.perfect_elimination_order.empty());
        const auto &cycle = result.induced_cycle;
        assert(cycle.size() >= 4);
        unsigned set = 0;
        for (int v : cycle)
        {
            assert(v >= 0 && v < n && ((set & (1U << v)) == 0));
            set |= 1U << v;
        }
        for (std::size_t i = 0; i < cycle.size(); ++i)
        {
            const int previous = cycle[(i + cycle.size() - 1) % cycle.size()];
            const int next = cycle[(i + 1) % cycle.size()];
            assert((adjacency[cycle[i]] & set) == ((1U << previous) | (1U << next)));
        }
    }
}

template <class Exception>
void expect_error(int n, std::initializer_list<Edge> edges)
{
    try
    {
        static_cast<void>(sirkit::recognize_chordal_graph(n, {edges.begin(), edges.size()}));
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
} // namespace

int main()
{
    for (int n = 0; n <= 6; ++n)
    {
        std::vector<Edge> possible;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u + 1; v < n; ++v)
            {
                possible.emplace_back(u, v);
            }
        }
        for (unsigned graph = 0; graph < (1U << possible.size()); ++graph)
        {
            std::vector<Edge> edges;
            for (auto left = graph; left != 0; left &= left - 1)
            {
                edges.push_back(possible[std::countr_zero(left)]);
            }
            check_small(n, edges);
        }
    }

    std::mt19937 random(830152);
    for (int n = 7; n <= 12; ++n)
    {
        for (int trial = 0; trial < 300; ++trial)
        {
            const auto density = random() % 101;
            std::vector<Edge> edges;
            for (int v = 0; v < n; ++v)
            {
                for (int u = 0; u < v; ++u)
                {
                    if (random() % 100 < density)
                    {
                        edges.emplace_back(u, v);
                        if (random() % 13 == 0)
                        {
                            edges.emplace_back(v, u);
                        }
                    }
                }
                if (random() % 17 == 0)
                {
                    edges.emplace_back(v, v);
                }
            }
            std::shuffle(edges.begin(), edges.end(), random);
            check_small(n, edges);
        }
    }
    check_small(7, {{0, 0}, {0, 1}, {1, 0}, {2, 3}, {3, 4}, {4, 5}, {5, 2}, {4, 3}});

    expect_error<std::length_error>(-1, {});
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::out_of_range>(2, {{-1, 0}});
    expect_error<std::out_of_range>(2, {{2, 0}});
    expect_error<std::out_of_range>(2, {{0, -1}});
    expect_error<std::out_of_range>(2, {{0, 2}});
#endif

    // 弦グラフの部分列では誘導閉路を検出しない。
    const std::vector<Edge> small_path{{0, 1}, {1, 2}, {2, 3}};
    const auto graph = sirkit::detail::make_chordal_adjacency(4, small_path);
    const auto order = sirkit::detail::chordal_maximum_cardinality_search(4, graph);
    assert(sirkit::detail::chordal_prefix_hole(graph, order, 3).empty());

    constexpr int kSize = 25000;
    std::vector<Edge> edges;
    for (int v = 1; v < kSize; ++v)
    {
        edges.emplace_back(v - 1, v);
    }
    auto result = sirkit::recognize_chordal_graph(kSize, edges);
    assert(result.is_chordal && result.perfect_elimination_order.size() == kSize);
    std::vector<int> rank(kSize, -1);
    for (int i = 0; i < kSize; ++i)
    {
        const auto v = result.perfect_elimination_order[i];
        assert(v >= 0 && v < kSize && rank[v] < 0);
        rank[v] = i;
    }
    for (int v = 1; v + 1 < kSize; ++v)
    {
        assert(rank[v] > std::min(rank[v - 1], rank[v + 1]));
    }

    edges.emplace_back(kSize - 1, 0);
    result = sirkit::recognize_chordal_graph(kSize, edges);
    assert(!result.is_chordal && result.induced_cycle.size() == kSize);
    std::vector<bool> used(kSize);
    for (int i = 0; i < kSize; ++i)
    {
        const auto v = result.induced_cycle[i];
        const auto to = result.induced_cycle[(i + 1) % kSize];
        assert(v >= 0 && v < kSize && !used[v]);
        used[v] = true;
        assert((v + 1) % kSize == to || (to + 1) % kSize == v);
    }
}
