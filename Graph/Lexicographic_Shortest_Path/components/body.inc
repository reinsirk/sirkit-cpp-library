#ifndef SIRKIT_LEXICOGRAPHIC_SHORTEST_PATH_DEFINED
#define SIRKIT_LEXICOGRAPHIC_SHORTEST_PATH_DEFINED

struct LexicographicShortestPath
{
    std::uint64_t distance;
    std::vector<int> vertices;
    std::vector<std::size_t> edges;
};

inline std::optional<LexicographicShortestPath> lexicographic_shortest_path(
    int n, std::span<const ShortestPathEdge> edges, int source, int target, bool undirected = false)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n || target < 0 || target >= n)
    {
        throw std::out_of_range("lexicographic_shortest_path: invalid terminal");
    }
#endif
    struct Arc
    {
        int to;
        std::size_t id;
    };
    std::vector<std::vector<Arc>> graph(n);
    detail::ShortestPathAdjacency reverse(n);
    auto add = [&](int u, int v, std::uint64_t weight, std::size_t id)
    {
        graph[u].push_back({v, id});
        reverse[v].emplace_back(weight, u);
    };
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v, weight] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("lexicographic_shortest_path: invalid endpoint");
        }
        if (weight == 0)
        {
            throw std::invalid_argument("lexicographic_shortest_path: weight must be positive");
        }
#endif
        add(u, v, weight, id);
        if (undirected && u != v)
        {
            add(v, u, weight, id);
        }
    }
    const auto distances = detail::shortest_path_edge_distances(reverse, target);
    if (!distances[source])
    {
        return std::nullopt;
    }
    LexicographicShortestPath result{*distances[source], {source}, {}};
    int u = source;
    while (u != target)
    {
        // 正の重みにより、最短路を保つ候補のどれに進んでも終点までの距離が減る。
        const auto remaining = *distances[u];
        std::optional<std::pair<int, std::size_t>> next;
        for (const auto &[v, id] : graph[u])
        {
            const auto weight = edges[id].weight;
            if (distances[v] && weight <= remaining && *distances[v] == remaining - weight)
            {
                const auto candidate = std::pair{v, id};
                if (!next || candidate < *next)
                {
                    next = candidate;
                }
            }
        }
        const auto [v, id] = next.value();
        result.vertices.push_back(v);
        result.edges.push_back(id);
        u = v;
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
