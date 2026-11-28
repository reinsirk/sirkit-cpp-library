// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Virtual_Tree.cpp"

#include <array>
#include <cassert>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

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

void check_nodes(const sirkit::VirtualTree &tree, const std::vector<Edge> &edges, int root,
                 const std::vector<int> &terminals)
{
    const auto actual = tree.build(terminals);
    if (terminals.empty())
    {
        assert(actual.empty());
        return;
    }
    const auto n = edges.size() + 1;
    std::vector<std::vector<int>> adjacency(n);
    for (const auto &[u, v] : edges)
    {
        adjacency[u].push_back(v);
        adjacency[v].push_back(u);
    }
    std::vector<int> queue{root}, parent(n, -1), depth(n);
    parent[root] = root;
    for (std::size_t i = 0; i < queue.size(); ++i)
    {
        const int v = queue[i];
        for (const int to : adjacency[v])
        {
            if (parent[to] == -1)
            {
                queue.push_back(to);
                parent[to] = v;
                depth[to] = depth[v] + 1;
            }
        }
    }
    const auto lca = [&](int a, int b) {
        while (depth[a] > depth[b])
        {
            a = parent[a];
        }
        while (depth[b] > depth[a])
        {
            b = parent[b];
        }
        while (a != b)
        {
            a = parent[a];
            b = parent[b];
        }
        return a;
    };
    const std::set<int> selected(terminals.begin(), terminals.end());
    auto expected = selected;
    for (const int a : selected)
    {
        for (const int b : selected)
        {
            expected.insert(lca(a, b));
        }
    }
    assert(actual.size() == expected.size());
    std::set<int> visited;
    for (std::size_t i = 0; i < actual.size(); ++i)
    {
        const auto &[v, p, length, terminal] = actual[i];
        assert(expected.contains(v) && visited.insert(v).second);
        assert(terminal == selected.contains(v));
        int toward_root = parent[v];
        while (toward_root != v && toward_root != parent[toward_root] && !expected.contains(toward_root))
        {
            toward_root = parent[toward_root];
        }
        if (toward_root == v || !expected.contains(toward_root))
        {
            assert(i == 0 && p == -1 && length == 0);
        }
        else
        {
            assert(p >= 0 && static_cast<std::size_t>(p) < i);
            assert(actual[p].vertex == toward_root && length == depth[v] - depth[toward_root]);
        }
    }
}
}

int main()
{
    const sirkit::VirtualTree empty(0, {});
    assert(empty.build({}).empty());
    must_throw<std::length_error>([] { const sirkit::VirtualTree tree(-1, {}); });
#ifndef SKIP_RUNTIME_CHECKS
    must_throw<std::invalid_argument>([] { const sirkit::VirtualTree tree(0, {}, 1); });
    must_throw<std::invalid_argument>([] {
        const std::array edges{Edge{0, 0}};
        const sirkit::VirtualTree tree(0, edges);
    });
    must_throw<std::invalid_argument>([] { const sirkit::VirtualTree tree(1, {}, -1); });
    must_throw<std::invalid_argument>([] { const sirkit::VirtualTree tree(1, {}, 1); });
    must_throw<std::invalid_argument>([] { const sirkit::VirtualTree tree(2, {}); });
    for (const auto &bad : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}, Edge{0, 0}})
    {
        must_throw<std::invalid_argument>([&] { const sirkit::VirtualTree tree(2, std::span(&bad, 1)); });
    }
    const std::array triangle{Edge{0, 1}, Edge{1, 2}, Edge{2, 0}};
    must_throw<std::invalid_argument>([&] { const sirkit::VirtualTree tree(4, triangle); });
    must_throw<std::invalid_argument>([&] { const sirkit::VirtualTree tree(4, triangle, 3); });
    const std::array parallel{Edge{0, 1}, Edge{1, 0}};
    must_throw<std::invalid_argument>([&] { const sirkit::VirtualTree tree(3, parallel); });
    const std::array invalid_negative{-1}, invalid_high{0};
    must_throw<std::out_of_range>([&] { static_cast<void>(empty.build(invalid_negative)); });
    must_throw<std::out_of_range>([&] { static_cast<void>(empty.build(invalid_high)); });
#endif

    std::mt19937 random(20261128);
    for (int n = 1; n <= 36; ++n)
    {
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            edges.emplace_back(v, static_cast<int>(random() % v));
        }
        for (int query = 0; query < 80; ++query)
        {
            const int root = static_cast<int>(random() % n);
            const sirkit::VirtualTree tree(n, edges, root);
            std::vector<int> terminals(random() % (n + 2));
            for (int &v : terminals)
            {
                v = static_cast<int>(random() % n);
            }
            check_nodes(tree, edges, root, terminals);
        }
    }
    constexpr int kLarge = 200000;
    std::vector<Edge> path;
    path.reserve(kLarge - 1);
    for (int v = 1; v < kLarge; ++v)
    {
        path.emplace_back(v - 1, v);
    }
    const sirkit::VirtualTree chain(kLarge, path);
    const std::array opposite{kLarge - 1, 0, kLarge - 1};
    const auto compressed = chain.build(opposite);
    assert(compressed.size() == 2 && compressed[1].distance == kLarge - 1);
}
