// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Nearest_Marked.cpp"

#include <cassert>
#include <optional>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Tree = sirkit::TreeNearestMarked;

std::vector<int> distances(const sirkit::Graph &graph, int root)
{
    std::vector<int> distance(graph.size(), -1);
    distance[root] = 0;
    std::queue<int> pending;
    pending.push(root);
    while (!pending.empty())
    {
        const int v = pending.front();
        pending.pop();
        for (const int to : graph[v])
        {
            if (distance[to] == -1)
            {
                distance[to] = distance[v] + 1;
                pending.push(to);
            }
        }
    }
    return distance;
}

std::optional<Tree::Result> best_mark(const std::vector<int> &distance, const std::vector<bool> &marked)
{
    std::optional<Tree::Result> best;
    for (std::size_t v = 0; v < distance.size(); ++v)
    {
        if (marked[v] && (!best || distance[v] < best->distance))
        {
            best = Tree::Result{static_cast<int>(v), distance[v]};
        }
    }
    return best;
}

void check_limits()
{
    assert(Tree({}).size() == 0);
    Tree singleton(sirkit::Graph(1));
    assert(singleton.size() == 1 && !singleton.marked(0) && singleton.nearest(0) == std::nullopt);
    singleton.set_marked(0);
    singleton.set_marked(0);
    assert((singleton.nearest(0) == Tree::Result{0, 0}));
    assert(!singleton.toggle(0));
    assert(!singleton.nearest(0));
    assert(singleton.toggle(0));
    assert(singleton.marked(0));
    singleton.set_marked(0, false);
    assert(!singleton.marked(0));
    Tree tie({{1, 2, 3}, {0}, {0}, {0}});
    tie.set_marked(3);
    tie.set_marked(2);
    assert((tie.nearest(1) == Tree::Result{2, 2}));
    assert((tie.nearest(1) != Tree::Result{2, 1}));
    assert((tie.nearest(1) != Tree::Result{3, 2}));
    tie.set_marked(0);
    assert((tie.nearest(1) == Tree::Result{0, 1}));
    tie.set_marked(0, false);
    tie.set_marked(2, false);
    assert((tie.nearest(1) == Tree::Result{3, 2}));
    Tree copied(tie);
    copied.set_marked(1);
    assert((copied.nearest(1) == Tree::Result{1, 0}));
    assert((tie.nearest(1) == Tree::Result{3, 2}));
    singleton = copied;
    assert((singleton.nearest(1) == Tree::Result{1, 0}));
    singleton = std::move(tie);
    assert((singleton.nearest(1) == Tree::Result{3, 2}));
#ifndef SKIP_RUNTIME_CHECKS
    for (const int v : {-1, 4})
    {
        bool caught = false;
        try
        {
            static_cast<void>(singleton.nearest(v));
        }
        catch (const std::out_of_range &)
        {
            caught = true;
        }
        assert(caught);
    }
#endif
}

void check_random()
{
    std::mt19937 rng(3189);
    for (int test = 0; test < 160; ++test)
    {
        const int n = 2 + static_cast<int>(rng() % 61);
        sirkit::Graph graph(n);
        for (int v = 1; v < n; ++v)
        {
            const int parent = static_cast<int>(rng() % v);
            graph[v].push_back(parent);
            graph[parent].push_back(v);
        }
        std::vector<std::vector<int>> distance(n);
        for (int u = 0; u < n; ++u)
        {
            distance[u] = distances(graph, u);
        }
        Tree tree(graph);
        std::vector<bool> marked(n);
        for (int step = 0; step < 250; ++step)
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
            const int query = static_cast<int>(rng() % n);
            assert(tree.nearest(query) == best_mark(distance[query], marked));
        }
    }
}

void check_long_path()
{
    constexpr int kSize = 100'000;
    sirkit::Graph path(kSize);
    for (int v = 1; v < kSize; ++v)
    {
        path[v].push_back(v - 1);
        path[v - 1].push_back(v);
    }
    Tree tree(path);
    tree.set_marked(kSize - 1);
    assert((tree.nearest(0) == Tree::Result{kSize - 1, kSize - 1}));
    tree.set_marked(0);
    assert((tree.nearest(kSize / 2 - 1) == Tree::Result{0, kSize / 2 - 1}));
    assert((tree.nearest(kSize / 2) == Tree::Result{kSize - 1, kSize / 2 - 1}));
    tree.set_marked(kSize - 1, false);
    assert((tree.nearest(kSize - 1) == Tree::Result{0, kSize - 1}));
}
}

int main()
{
    check_limits();
    check_random();
    check_long_path();
}
