#ifndef SIRKIT_WALK_WEIGHT_RANGES_DEFINED
#define SIRKIT_WALK_WEIGHT_RANGES_DEFINED

struct WalkWeightEdge
{
    int from, to;
    std::int32_t weight;
};

struct WalkWeightRange
{
    bool reachable = false;
    bool unbounded = false;
    std::int64_t minimum = 0;
    std::int64_t maximum = 0;
};

inline std::vector<WalkWeightRange> walk_weight_ranges(int n, std::span<const WalkWeightEdge> edges, int source)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n)
    {
        throw std::out_of_range("walk weight ranges: source out of range");
    }
#endif
    scc_graph graph(n);
    graph.reserve_edges(edges.size());
    std::vector<std::vector<std::pair<int, std::int32_t>>> adjacent(n);
    for (const auto &[u, v, weight] : edges)
    {
        graph.add_edge(u, v);
        adjacent[u].emplace_back(v, weight);
    }
    const auto groups = graph.scc();
    const auto &id = graph.ids();

    // 成分内で辺の重みをポテンシャル差として表せない場合、非零の閉じた歩道がある。
    std::vector<bool> nonzero_cycle(groups.size()), entered(n);
    std::vector<std::int64_t> potential(n);
    std::vector<int> pending;
    for (std::size_t component = 0; component < groups.size(); ++component)
    {
        const int root = groups[component].front();
        entered[root] = true;
        pending.push_back(root);
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const int u = pending[i];
            for (const auto &[v, weight] : adjacent[u])
            {
                if (id[v] != static_cast<int>(component))
                {
                    continue;
                }
                const auto candidate = potential[u] + weight;
                if (!entered[v])
                {
                    entered[v] = true;
                    potential[v] = candidate;
                    pending.push_back(v);
                }
                else if (candidate != potential[v])
                {
                    nonzero_cycle[component] = true;
                }
            }
        }
        pending.clear();
    }

    // SCCの番号はトポロジカル順。代表点の最小・最大値を成分間の辺で更新する。
    std::vector<WalkWeightRange> states(groups.size());
    states[id[source]] = {true, false, -potential[source], -potential[source]};
    for (std::size_t component = 0; component < groups.size(); ++component)
    {
        auto &range = states[component];
        if (!range.reachable)
        {
            continue;
        }
        if (nonzero_cycle[component])
        {
            range = {true, true, 0, 0};
        }
        for (const int u : groups[component])
        {
            for (const auto &[v, weight] : adjacent[u])
            {
                if (id[v] == static_cast<int>(component))
                {
                    continue;
                }
                auto &target = states[id[v]];
                if (range.unbounded)
                {
                    target = {true, true, 0, 0};
                }
                else if (!target.unbounded)
                {
                    const auto shift = potential[u] + weight - potential[v];
                    const auto low = range.minimum + shift;
                    const auto high = range.maximum + shift;
                    if (!target.reachable)
                    {
                        target = {true, false, low, high};
                    }
                    else
                    {
                        target.minimum = std::min(target.minimum, low);
                        target.maximum = std::max(target.maximum, high);
                    }
                }
            }
        }
    }

    std::vector<WalkWeightRange> result(n);
    for (int vertex = 0; vertex < n; ++vertex)
    {
        result[vertex] = states[id[vertex]];
        if (result[vertex].reachable && !result[vertex].unbounded)
        {
            result[vertex].minimum += potential[vertex];
            result[vertex].maximum += potential[vertex];
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
