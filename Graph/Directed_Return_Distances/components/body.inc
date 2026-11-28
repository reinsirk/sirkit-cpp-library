#ifndef SIRKIT_DIRECTED_RETURN_DISTANCES_DEFINED
#define SIRKIT_DIRECTED_RETURN_DISTANCES_DEFINED

struct ReturnDistanceArc
{
    int from;
    int to;
    std::uint64_t weight;
};

[[nodiscard]] inline std::vector<std::optional<std::uint64_t>>
directed_return_distances(int n, std::span<const ReturnDistanceArc> edges)
{
    scc_graph components(n);
    components.reserve_edges(edges.size());
    for (const auto &[u, v, weight] : edges)
    {
        components.add_edge(u, v);
    }
    const int count = components.build();
    const auto &id = components.ids();

    using Arc = std::pair<int, std::uint64_t>;
    std::vector<std::vector<Arc>> adjacent(n);
    for (const auto &[u, v, weight] : edges)
    {
        if (id[u] == id[v])
        {
            adjacent[u].emplace_back(v, weight);
        }
    }

    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    std::vector<bool> simple_cycle(count, true);
    std::vector<std::optional<std::uint64_t>> total(count, 0);
    for (int u = 0; u < n; ++u)
    {
        if (adjacent[u].size() != 1)
        {
            simple_cycle[id[u]] = false;
        }
        else if (auto &current = total[id[u]]; current)
        {
            const auto weight = adjacent[u].front().second;
            if (weight > kMax - *current)
            {
                current.reset();
            }
            else
            {
                *current += weight;
            }
        }
    }

    using State = std::pair<std::uint64_t, int>;
    constexpr std::greater<State> kGreater;
    std::vector<State> pending;
    std::vector<int> stamp(n, -1);
    std::vector<std::uint64_t> distance(n);
    std::vector<std::optional<std::uint64_t>> result(n);
    for (int source = 0; source < n; ++source)
    {
        if (adjacent[source].empty())
        {
            continue;
        }
        if (simple_cycle[id[source]])
        {
            result[source] = total[id[source]];
            continue;
        }
        auto push = [&](int v, std::uint64_t value)
        {
            if (stamp[v] != source || value < distance[v])
            {
                stamp[v] = source;
                distance[v] = value;
                pending.emplace_back(value, v);
                std::ranges::push_heap(pending, kGreater);
            }
        };

        pending.clear();
        for (const auto &[v, weight] : adjacent[source])
        {
            push(v, weight);
        }
        while (!pending.empty())
        {
            std::ranges::pop_heap(pending, kGreater);
            const auto [current, u] = pending.back();
            pending.pop_back();
            if (current != distance[u])
            {
                continue;
            }
            if (u == source)
            {
                result[source] = current;
                break;
            }
            for (const auto &[v, weight] : adjacent[u])
            {
                if (weight <= kMax - current)
                {
                    push(v, current + weight);
                }
            }
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
