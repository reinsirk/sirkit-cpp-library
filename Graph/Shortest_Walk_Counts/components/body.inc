#ifndef SIRKIT_SHORTEST_WALK_COUNTS_DEFINED
#define SIRKIT_SHORTEST_WALK_COUNTS_DEFINED

template <class Count>
struct ShortestWalkCounts
{
    std::vector<std::optional<std::uint64_t>> distance;
    std::vector<Count> count;
    std::vector<bool> infinite;
};

template <class Count = std::uint64_t>
ShortestWalkCounts<Count> count_shortest_walks(
    int n, std::span<const ShortestPathEdge> edges, int source, bool undirected = false)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n)
    {
        throw std::out_of_range("shortest walk counts: source out of range");
    }
#endif
    detail::ShortestPathAdjacency graph(n);
    bool unit = true;
    for (const auto &[u, v, weight] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("shortest walk counts: endpoint out of range");
        }
#endif
        unit &= weight == 1;
        graph[u].emplace_back(weight, v);
        if (undirected && u != v)
        {
            graph[v].emplace_back(weight, u);
        }
    }

    ShortestWalkCounts<Count> result{{}, std::vector<Count>(n, Count{0}), std::vector<bool>(n)};
    result.count[source] = Count{1};
    std::vector<int> pending;
    pending.reserve(n);
    if (unit)
    {
        result.distance.resize(n);
        result.distance[source] = 0;
        pending.push_back(source);
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const int u = pending[i];
            const auto next = *result.distance[u] + 1;
            for (const auto &[weight, v] : graph[u])
            {
                if (!result.distance[v])
                {
                    result.distance[v] = next;
                    pending.push_back(v);
                }
                if (*result.distance[v] == next)
                {
                    result.count[v] += result.count[u];
                }
            }
        }
        return result;
    }

    result.distance = detail::shortest_path_edge_distances(graph, source);
    std::vector<std::vector<int>> tight(n);
    std::vector<std::size_t> incoming(n);
    for (int u = 0; u < n; ++u)
    {
        const auto prefix = result.distance[u];
        if (!prefix)
        {
            continue;
        }
        for (const auto &[weight, v] : graph[u])
        {
            const auto finish = result.distance[v];
            if (finish && *prefix <= *finish && weight == *finish - *prefix)
            {
                tight[u].push_back(v);
                ++incoming[v];
            }
        }
    }

    for (int u = 0; u < n; ++u)
    {
        if (result.distance[u] && incoming[u] == 0)
        {
            pending.push_back(u);
        }
    }
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const int u = pending[i];
        for (const int v : tight[u])
        {
            result.count[v] += result.count[u];
            if (--incoming[v] == 0)
            {
                pending.push_back(v);
            }
        }
    }
    // 入次数が残る頂点へは、最短な歩道の途中で重み0の閉路を繰り返せる。
    for (int u = 0; u < n; ++u)
    {
        if (incoming[u] != 0)
        {
            result.infinite[u] = true;
            result.count[u] = Count{0};
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
