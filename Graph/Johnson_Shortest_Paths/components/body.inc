#ifndef SIRKIT_JOHNSON_SHORTEST_PATHS_DEFINED
#define SIRKIT_JOHNSON_SHORTEST_PATHS_DEFINED

struct JohnsonEdge
{
    int from;
    int to;
    std::int64_t weight;
};

template <typename Consumer>
bool for_each_johnson_distance_row(int n, std::span<const JohnsonEdge> edges, Consumer &&on_row)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::length_error("for_each_johnson_distance_row: negative vertex count");
    }
    const auto cap = (std::numeric_limits<std::int64_t>::max() / 4) / std::max(1, n);
#endif
    bool needs_potentials = false;
    for (const auto &[u, v, weight] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("for_each_johnson_distance_row: invalid endpoint");
        }
        if (weight < -cap || weight > cap)
        {
            throw std::out_of_range("for_each_johnson_distance_row: weight too large for the vertex count");
        }
#endif
        needs_potentials = needs_potentials || weight < 0;
    }

    // 全頂点に重み0で届く仮の始点を、初期値0で表す。
    std::vector<std::int64_t> potential(n);
    if (needs_potentials)
    {
        std::vector<std::int64_t> next(n);
        for (int remaining = n;; --remaining)
        {
            next = potential;
            bool improved = false;
            for (const auto &[u, v, weight] : edges)
            {
                const auto candidate = potential[u] + weight;
                if (candidate < next[v])
                {
                    next[v] = candidate;
                    improved = true;
                }
            }
            if (!improved)
            {
                break;
            }
            if (remaining == 1)
            {
                return false;
            }
            potential.swap(next);
        }
    }

    detail::ShortestPathAdjacency graph(n);
    for (const auto &[u, v, weight] : edges)
    {
        const auto nonnegative = weight + potential[u] - potential[v];
        graph[u].emplace_back(static_cast<std::uint64_t>(nonnegative), v);
    }
    std::vector<std::optional<std::int64_t>> row(n);
    for (int u = 0; u < n; ++u)
    {
        const auto reweighted = detail::shortest_path_edge_distances(graph, u);
        for (int v = 0; v < n; ++v)
        {
            if (reweighted[v])
            {
                row[v] = static_cast<std::int64_t>(*reweighted[v]) - potential[u] + potential[v];
            }
            else
            {
                row[v] = std::nullopt;
            }
        }
        std::invoke(on_row, u, std::span<const std::optional<std::int64_t>>(row));
    }
    return true;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
