// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Unrooted_Tree_Isomorphism.cpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;
using Tree = std::vector<Edge>;

std::string shape(int n, const Tree &edges, int cut = -1)
{
    std::vector<std::vector<int>> graph(n);
    for (const auto &[u, v] : edges)
    {
        if (u != cut && v != cut)
        {
            graph[u].push_back(v);
            graph[v].push_back(u);
        }
    }
    auto trace = [&](auto &self, int v, int parent) -> std::string
    {
        std::vector<std::string> children;
        for (int w : graph[v])
        {
            if (w != parent)
            {
                children.push_back(self(self, w, v));
            }
        }
        std::sort(children.begin(), children.end());
        std::string signature("(");
        for (const auto &child : children)
        {
            signature += child;
        }
        signature += ')';
        return signature;
    };
    std::string best;
    for (int v = 0; v < n; ++v)
    {
        if (v != cut)
        {
            const auto signature = trace(trace, v, -1);
            if (best.empty() || signature < best)
            {
                best = signature;
            }
        }
    }
    return best;
}

void check_leaves(int n, const Tree &tree, const Tree &target)
{
    const auto expected = shape(n - 1, target);
    std::vector<int> leaves, degree(n);
    for (const auto &[u, v] : tree)
    {
        ++degree[u];
        ++degree[v];
    }
    for (int v = 0; v < n; ++v)
    {
        if (degree[v] == 1 && shape(n, tree, v) == expected)
        {
            leaves.push_back(v);
        }
    }
    assert(sirkit::tree_leaf_deletion_matches(n, tree, target) == leaves);
}

Tree relabel(int n, Tree edges, std::mt19937 &random)
{
    std::vector<int> names(n);
    std::iota(names.begin(), names.end(), 0);
    std::shuffle(names.begin(), names.end(), random);
    for (auto &[u, v] : edges)
    {
        u = names[u];
        v = names[v];
        if (random() % 2)
        {
            std::swap(u, v);
        }
    }
    std::shuffle(edges.begin(), edges.end(), random);
    return edges;
}

Tree random_tree(int n, std::mt19937 &random)
{
    Tree edges;
    for (int v = 1; v < n; ++v)
    {
        edges.emplace_back(v, random() % v);
    }
    return relabel(n, std::move(edges), random);
}

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
} // namespace

int main()
{
    assert(sirkit::unrooted_trees_equal(0, {}, {}));
    assert(sirkit::unrooted_trees_equal(1, {}, {}));
    const Tree unit{{0, 1}};
    check_leaves(2, unit, {});
    const Tree path{{0, 1}, {1, 2}, {2, 3}, {3, 4}};
    const Tree star{{0, 1}, {0, 2}, {0, 3}, {0, 4}};
    assert(!sirkit::unrooted_trees_equal(5, path, star));
    check_leaves(5, path, {{0, 1}, {1, 2}, {2, 3}});
    check_leaves(5, star, {{0, 1}, {0, 2}, {0, 3}});
    check_leaves(5, path, {{0, 1}, {0, 2}, {0, 3}});
    check_leaves(5, star, {{0, 1}, {1, 2}, {2, 3}});
    constexpr int kN = 40000;
    Tree a(kN - 1), b(kN - 2);
    for (int v = 0; v < kN - 1; ++v)
    {
        a[v] = {v, v + 1};
        if (v + 1 < kN - 1)
        {
            b[v] = {v, v + 1};
        }
    }
    assert((sirkit::tree_leaf_deletion_matches(kN, a, b) == std::vector<int>{0, kN - 1}));

    std::mt19937 random(201927);
    for (int test = 0; test < 1300; ++test)
    {
        const int n = 2 + static_cast<int>(random() % 13);
        const auto first = random_tree(n, random);
        const auto second = random_tree(n, random);
        assert(sirkit::unrooted_trees_equal(n, first, second) == (shape(n, first) == shape(n, second)));
        assert(sirkit::unrooted_trees_equal(n, first, relabel(n, first, random)));
        check_leaves(n, first, random_tree(n - 1, random));
        std::vector<int> degree(n);
        for (const auto &[u, v] : first)
        {
            ++degree[u];
            ++degree[v];
        }
        const int leaf = static_cast<int>(std::find(degree.begin(), degree.end(), 1) - degree.begin());
        Tree reduced;
        for (const auto &[u, v] : first)
        {
            if (u != leaf && v != leaf)
            {
                reduced.emplace_back(u - (u > leaf), v - (v > leaf));
            }
        }
        check_leaves(n, first, relabel(n - 1, std::move(reduced), random));
    }

    check_error<std::length_error>([] { (void)sirkit::unrooted_trees_equal(-1, {}, {}); });
    check_error<std::length_error>([] { (void)sirkit::tree_leaf_deletion_matches(std::numeric_limits<int>::max(), {}, {}); });
    check_error<std::invalid_argument>([] { (void)sirkit::tree_leaf_deletion_matches(1, {}, {}); });
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::invalid_argument>([] { (void)sirkit::unrooted_trees_equal(2, {}, {}); });
    for (const Edge &invalid : std::vector<Edge>{{-1, 0}, {2, 0}, {0, -1}, {0, 2}})
    {
        check_error<std::out_of_range>([&] { (void)sirkit::unrooted_trees_equal(2, unit, std::span(&invalid, 1)); });
    }
    const Tree cycle{{0, 1}, {0, 2}, {1, 2}};
    const Tree disconnected_cycle{{1, 2}, {1, 3}, {2, 3}};
    const Tree simple{{0, 1}, {1, 2}, {2, 3}};
    check_error<std::invalid_argument>([&] { (void)sirkit::unrooted_trees_equal(4, cycle, simple); });
    check_error<std::invalid_argument>([&] { (void)sirkit::unrooted_trees_equal(4, simple, disconnected_cycle); });
#endif
}
