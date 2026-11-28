// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Mo.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <queue>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Pair = std::pair<int, int>;
using Marked = std::vector<std::uint8_t>;
using Graph = std::vector<std::vector<std::pair<int, std::size_t>>>;

template<class Exception, class Operation>
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

std::pair<Marked, Marked> reference_path(const Graph &graph, int source, int target)
{
    const auto n = graph.size();
    std::vector<int> parent(n, -1);
    std::vector<std::size_t> edge_at(n);
    std::queue<int> pending;
    pending.push(source);
    parent[source] = source;
    while (!pending.empty())
    {
        const int v = pending.front();
        pending.pop();
        for (const auto &[u, edge] : graph[v])
        {
            if (parent[u] == -1)
            {
                parent[u] = v;
                edge_at[u] = edge;
                pending.push(u);
            }
        }
    }
    Marked nodes(n), edges(n - 1);
    nodes[source] = 1;
    for (int v = target; v != source; v = parent[v])
    {
        nodes[v] = 1;
        edges[edge_at[v]] = 1;
    }
    return {nodes, edges};
}

void compare(const sirkit::TreeMo &tree, std::span<const Pair> queries,
             const std::vector<Marked> &expected, bool edge_mode, std::int32_t width)
{
    Marked active(tree.size() - edge_mode), seen(queries.size());
    const auto add = [&](std::size_t id)
    {
        assert(active[id] == 0);
        active[id] = 1;
    };
    const auto erase = [&](std::size_t id)
    {
        assert(active[id] == 1);
        active[id] = 0;
    };
    const auto answer = [&](std::size_t id)
    {
        assert(seen[id] == 0);
        seen[id] = 1;
        assert(active == expected[id]);
    };
    if (edge_mode)
    {
        tree.run_edges(queries, add, erase, answer, width);
    }
    else
    {
        tree.run(queries, add, erase, answer, width);
    }
    assert(std::count(seen.begin(), seen.end(), 1) == static_cast<std::ptrdiff_t>(queries.size()));
    assert(std::count(active.begin(), active.end(), 1) == 0);
}

void check_limits()
{
    const sirkit::TreeMo empty(0, {});
    assert(empty.size() == 0);
    const auto never = [](std::size_t) { assert(false); };
    empty.run({}, never, never, never);
    empty.run_edges({}, never, never, never);
    const sirkit::TreeMo singleton(1, {});
    const std::array query{Pair{0, 0}};
    compare(singleton, query, {Marked{1}}, false, 0);
    compare(singleton, query, {Marked{}}, true, 1);
    const std::array inverted{Pair{2, 1}, Pair{1, 0}};
    sirkit::TreeMo trial(3, inverted, 2);
    const std::array full{Pair{2, 0}};
    compare(trial, full, {Marked{1, 1, 1}}, false, 0);
    compare(trial, full, {Marked{1, 1}}, true, 2);
    sirkit::TreeMo copy(trial);
    trial = singleton;
    compare(trial, query, {Marked{1}}, false, 3);
    trial = std::move(copy);
    compare(trial, full, {Marked{1, 1}}, true, 2);

    must_throw<std::length_error>([] { const sirkit::TreeMo invalid(-1, {}); });
    must_throw<std::length_error>([] {
        const sirkit::TreeMo over(std::numeric_limits<std::int32_t>::max() / 2 + 1, {});
    });
#ifndef SKIP_RUNTIME_CHECKS
    must_throw<std::invalid_argument>([] { const sirkit::TreeMo disconnected(2, {}); });
    for (const auto &bad : {Pair{-1, 0}, Pair{0, -1}, Pair{1, 0}, Pair{0, 1}})
    {
        must_throw<std::out_of_range>([&] { singleton.run(std::span(&bad, 1), never, never, never); });
        must_throw<std::out_of_range>([&] { singleton.run_edges(std::span(&bad, 1), never, never, never); });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(477);
    for (int test = 0; test < 170; ++test)
    {
        const int n = 2 + static_cast<int>(rng() % 65);
        Graph graph(n);
        std::vector<Pair> edges;
        for (int v = 1; v < n; ++v)
        {
            const int parent = static_cast<int>(rng() % v);
            const auto edge = edges.size();
            graph[v].emplace_back(parent, edge);
            graph[parent].emplace_back(v, edge);
            edges.push_back(v % 2 == 0 ? Pair{v, parent} : Pair{parent, v});
        }
        const sirkit::TreeMo tree(n, edges, static_cast<int>(rng() % n));
        std::vector<Pair> queries;
        std::vector<Marked> nodes, arcs;
        for (int q = 0; q < 115; ++q)
        {
            const int from = static_cast<int>(rng() % n), to = static_cast<int>(rng() % n);
            queries.emplace_back(from, to);
            auto [a, b] = reference_path(graph, from, to);
            nodes.push_back(std::move(a));
            arcs.push_back(std::move(b));
        }
        compare(tree, queries, nodes, false, test % 9);
        compare(tree, queries, arcs, true, test % 11);
    }
}
}

int main()
{
    check_limits();
    check_random();
}
