#ifndef SIRKIT_BIPARTITE_COLORING_DEFINED
#define SIRKIT_BIPARTITE_COLORING_DEFINED

struct BipartiteColoring
{
    std::vector<std::int8_t> color;
    std::optional<Cycle> odd_cycle;

    [[nodiscard]] bool is_bipartite() const noexcept
    {
        return !odd_cycle;
    }
};

inline BipartiteColoring bipartite_coloring(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("bipartite_coloring: negative size");
    }
    struct Arc
    {
        int to;
        std::size_t id;
    };
    std::vector<std::vector<Arc>> graph(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("bipartite_coloring: invalid endpoint");
        }
#endif
        graph[u].push_back({v, id});
        graph[v].push_back({u, id});
    }

    std::vector<std::int8_t> color(n, -1);
    std::vector<int> parent(n, -1);
    std::vector<std::size_t> parent_edge(n);
    std::vector<int> pending;
    pending.reserve(n);
    for (int root = 0; root < n; ++root)
    {
        if (color[root] != -1)
        {
            continue;
        }
        color[root] = 0;
        pending.clear();
        pending.push_back(root);
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const int u = pending[i];
            for (const auto &[v, id] : graph[u])
            {
                if (color[v] == -1)
                {
                    color[v] = static_cast<std::int8_t>(1 - color[u]);
                    parent[v] = u;
                    parent_edge[v] = id;
                    pending.push_back(v);
                }
                else if (color[v] == color[u])
                {
                    Cycle left, right;
                    int a = u;
                    int b = v;
                    // Adjacent vertices of the same color are at the same BFS depth.
                    while (a != b)
                    {
                        left.vertices.push_back(a);
                        left.edge_ids.push_back(parent_edge[a]);
                        right.vertices.push_back(b);
                        right.edge_ids.push_back(parent_edge[b]);
                        a = parent[a];
                        b = parent[b];
                    }
                    left.vertices.push_back(a);
                    std::ranges::reverse(left.vertices);
                    std::ranges::reverse(left.edge_ids);
                    left.edge_ids.push_back(id);
                    left.vertices.insert(left.vertices.end(), right.vertices.begin(), right.vertices.end());
                    left.edge_ids.insert(left.edge_ids.end(), right.edge_ids.begin(), right.edge_ids.end());
                    return {{}, std::move(left)};
                }
            }
        }
    }
    return {std::move(color), std::nullopt};
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
