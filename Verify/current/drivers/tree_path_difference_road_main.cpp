// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<std::pair<int, int>> edges;
    edges.reserve(n - 1);
    std::vector<std::int64_t> weights(n);
    sirkit::Graph graph(n);
    for (int v = 1; v < n; ++v)
    {
        int parent{};
        verify_in >> parent >> weights[v];
        --parent;
        edges.emplace_back(parent, v);
        graph[parent].push_back(v);
        graph[v].push_back(parent);
    }
    sirkit::TreePathDifference<> paths(n, edges);
    int updates{};
    verify_in >> updates;
    for (int i = 0; i < updates; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        paths.add_edge_path(u - 1, v - 1, 1);
    }
    const auto increments = paths.edge_values();
    for (int v = 1; v < n; ++v)
    {
        weights[v] += increments[v - 1];
    }
    const sirkit::HeavyLightDecomposition hld(graph);
    constexpr auto minimum = [](std::int64_t a, std::int64_t b) { return std::min(a, b); };
    constexpr auto identity = [] { return std::numeric_limits<std::int64_t>::max(); };
    const sirkit::segtree<std::int64_t, minimum, identity> range(hld.reorder(weights));
    int questions{};
    verify_in >> questions;
    for (int i = 0; i < questions; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        verify_out << (u == v ? 0 : hld.path_prod(u - 1, v - 1, range, true)) << '\n';
    }
}
