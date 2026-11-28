// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Dynamic_Steiner_Subtree.cpp"

#include <array>
#include <cassert>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Tree = sirkit::DynamicSteinerSubtree;
using Edges = std::vector<std::pair<int, int>>;
using Graph = std::vector<std::vector<int>>;

std::vector<bool> terminals_in_subtree(const Graph &graph, const std::vector<bool> &marked, int v, int root)
{
    std::vector<int> parent(graph.size(), -2);
    parent[root] = -1;
    std::vector<int> pending{root};
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const int here = pending[i];
        for (const int next : graph[here])
        {
            if (next != parent[here])
            {
                parent[next] = here;
                pending.push_back(next);
            }
        }
    }
    std::vector<bool> terminals(graph.size());
    for (std::size_t i = 0; i < graph.size(); ++i)
    {
        if (marked[i])
        {
            for (int ancestor = static_cast<int>(i); ancestor != -1; ancestor = parent[ancestor])
            {
                if (ancestor == v)
                {
                    terminals[i] = true;
                    break;
                }
            }
        }
    }
    return terminals;
}

int prune_nonterminal_leaves(const Graph &graph, const std::vector<bool> &terminals)
{
    const int n = static_cast<int>(graph.size());
    std::vector<int> degree(n);
    std::queue<int> leaves;
    for (int v = 0; v < n; ++v)
    {
        degree[v] = static_cast<int>(graph[v].size());
        if (degree[v] <= 1 && !terminals[v])
        {
            leaves.push(v);
        }
    }
    int alive = n;
    while (!leaves.empty())
    {
        const int v = leaves.front();
        leaves.pop();
        if (degree[v] < 0)
        {
            continue;
        }
        degree[v] = -1;
        --alive;
        for (const int neighbor : graph[v])
        {
            if (degree[neighbor] > 0 && --degree[neighbor] <= 1 && !terminals[neighbor])
            {
                leaves.push(neighbor);
            }
        }
    }
    return alive;
}

void check_limits()
{
    const Tree empty(0, {});
    assert(empty.size() == 0 && empty.steiner_vertices() == 0);
    Tree singleton(1, {});
    assert(singleton.steiner_vertices() == 0);
    singleton.set_marked(0, false);
    assert(singleton.toggle(0));
    assert(singleton.steiner_vertices(0, 0) == 1);
    singleton.set_marked(0);
    assert(singleton.marked(0));
    assert(!singleton.toggle(0));
    assert(singleton.steiner_vertices(0, 0) == 0);

    const Edges edges{{0, 1}, {1, 2}, {1, 3}};
    Tree tree(4, edges, 3);
    tree.set_marked(0);
    tree.set_marked(2);
    Tree copy(tree);
    copy.set_marked(3);
    assert(tree.steiner_vertices() == 3);
    assert(copy.steiner_vertices() == 4);
    singleton = copy;
    assert(singleton.steiner_vertices() == 4);
    singleton = std::move(tree);
    assert(tree.steiner_vertices() == 0);
    assert(singleton.steiner_vertices() == 3);
    assert(singleton.steiner_vertices(2, 3) == 1);
    assert(singleton.steiner_vertices(2, 0) == 1);
    assert(singleton.steiner_vertices(1, 0) == 1);

#ifndef SKIP_RUNTIME_CHECKS
    bool caught = false;
    try
    {
        const std::array forest{std::pair{0, 1}};
        const Tree disconnected(3, forest);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught);
#endif
}

void check_random()
{
    std::mt19937 rng(3442);
    for (int test = 0; test < 190; ++test)
    {
        const int n = 1 + static_cast<int>(rng() % 41);
        Edges edges;
        Graph graph(n);
        for (int v = 1; v < n; ++v)
        {
            const int parent = static_cast<int>(rng() % v);
            edges.emplace_back(parent, v);
            graph[parent].push_back(v);
            graph[v].push_back(parent);
        }
        Tree tree(n, edges, static_cast<int>(rng() % n));
        std::vector<bool> marked(n);
        for (int step = 0; step < 180; ++step)
        {
            const int v = static_cast<int>(rng() % n);
            if (step % 3 == 0)
            {
                marked[v] = !marked[v];
                assert(tree.toggle(v) == marked[v]);
            }
            else
            {
                marked[v] = step % 3 == 1;
                tree.set_marked(v, marked[v]);
            }
            assert(tree.marked(v) == marked[v]);
            assert(tree.steiner_vertices() == prune_nonterminal_leaves(graph, marked));
            const int w = static_cast<int>(rng() % n);
            const int root = static_cast<int>(rng() % n);
            const auto terminals = terminals_in_subtree(graph, marked, w, root);
            assert(tree.steiner_vertices(w, root) == prune_nonterminal_leaves(graph, terminals));
        }
    }
}

void check_long_path()
{
    constexpr int kSize = 200'000;
    Edges path;
    path.reserve(kSize - 1);
    for (int v = 1; v < kSize; ++v)
    {
        path.emplace_back(v - 1, v);
    }
    Tree tree(kSize, path);
    tree.set_marked(3);
    tree.set_marked(60000);
    tree.set_marked(kSize - 1);
    assert(tree.steiner_vertices() == kSize - 3);
    assert(tree.steiner_vertices(123, 0) == kSize - 60000);
    assert(tree.steiner_vertices(60000, kSize - 1) == 60000 - 3 + 1);
    tree.set_marked(3, false);
    assert(tree.steiner_vertices(60000, kSize - 1) == 1);
}
}

int main()
{
    check_limits();
    check_random();
    check_long_path();
}
