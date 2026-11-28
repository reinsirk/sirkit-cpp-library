#ifndef SIRKIT_SHORTEST_WEIGHTED_CYCLE_DEFINED
#define SIRKIT_SHORTEST_WEIGHTED_CYCLE_DEFINED

struct WeightedCycleEdge
{
    int from;
    int to;
    std::uint64_t weight = 1;
};

struct WeightedCycle
{
    std::uint64_t weight;
    Cycle cycle;
};

namespace detail
{
struct WeightedCycleArc
{
    int to;
    std::size_t id;
    std::uint64_t weight;
};

using WeightedCycleGraph = std::vector<std::vector<WeightedCycleArc>>;

inline std::optional<WeightedCycle> shortest_weighted_cycle_from(
    const WeightedCycleGraph &graph, int source, std::size_t edge_count, bool directed,
    std::optional<std::uint64_t> limit)
{
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    struct Connection
    {
        int from;
        int to;
        std::size_t id;
        std::uint64_t cost;
    };
    std::optional<Connection> best;
    auto consider = [&](int from, int to, std::size_t id, std::uint64_t cost)
    {
        if (!limit || cost < *limit)
        {
            best = {from, to, id, cost};
            limit = cost;
        }
    };

    std::vector<std::optional<std::uint64_t>> distance(graph.size());
    std::vector<int> parent(graph.size(), -1);
    std::vector<std::size_t> incoming(graph.size());
    std::vector<std::size_t> first(graph.size(), edge_count);
    std::vector<bool> settled(graph.size());
    using State = std::pair<std::uint64_t, int>;
    std::priority_queue<State, std::vector<State>, std::greater<State>> queue;
    distance[source] = 0;
    queue.emplace(0, source);

    while (!queue.empty())
    {
        const auto [prefix, from] = queue.top();
        queue.pop();
        if (prefix != distance[from].value())
        {
            continue;
        }
        if (limit && prefix >= *limit)
        {
            break;
        }
        settled[from] = true;
        for (const auto &[to, id, weight] : graph[from])
        {
            if (weight > kMax - prefix)
            {
                continue;
            }
            const auto candidate = prefix + weight;
            if (to == source)
            {
                if (directed || first[from] != id)
                {
                    consider(from, to, id, candidate);
                }
                continue;
            }
            // In an undirected graph, two different first edges identify disjoint tree paths from source.
            if (!directed && from != source && settled[to] && first[from] != first[to])
            {
                const auto suffix = distance[to].value();
                if (suffix <= kMax - candidate)
                {
                    consider(from, to, id, candidate + suffix);
                }
            }
            if (!distance[to] || candidate < distance[to].value())
            {
                distance[to] = candidate;
                parent[to] = from;
                incoming[to] = id;
                first[to] = from == source ? id : first[from];
                queue.emplace(candidate, to);
            }
        }
    }
    if (!best)
    {
        return std::nullopt;
    }

    WeightedCycle result{best->cost, {}};
    auto &vertices = result.cycle.vertices;
    auto &edges = result.cycle.edge_ids;
    for (int v = best->from; v != source; v = parent[v])
    {
        vertices.push_back(v);
        edges.push_back(incoming[v]);
    }
    vertices.push_back(source);
    std::ranges::reverse(vertices);
    std::ranges::reverse(edges);
    edges.push_back(best->id);
    for (int v = best->to; v != source; v = parent[v])
    {
        vertices.push_back(v);
        edges.push_back(incoming[v]);
    }
    return result;
}
} // namespace detail

inline std::optional<WeightedCycle> shortest_weighted_cycle(
    int n, std::span<const WeightedCycleEdge> edges, bool directed = true,
    std::optional<int> through = std::nullopt)
{
    if (n < 0)
    {
        throw std::length_error("shortest_weighted_cycle: negative vertex count");
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (through && (*through < 0 || *through >= n))
    {
        throw std::out_of_range("shortest_weighted_cycle: invalid required vertex");
    }
#endif
    detail::WeightedCycleGraph graph(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [from, to, weight] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("shortest_weighted_cycle: invalid endpoint");
        }
#endif
        graph[from].push_back({to, id, weight});
        if (!directed)
        {
            graph[to].push_back({from, id, weight});
        }
    }

    std::optional<WeightedCycle> result;
    const int first = through.value_or(0);
    const int last = through ? *through + 1 : n;
    for (int v = first; v < last; ++v)
    {
        if (graph[v].empty())
        {
            continue;
        }
        const auto bound = result ? std::optional<std::uint64_t>(result->weight) : std::nullopt;
        if (auto better = detail::shortest_weighted_cycle_from(graph, v, edges.size(), directed, bound))
        {
            result = std::move(better);
            if (result->weight == 0)
            {
                break;
            }
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
