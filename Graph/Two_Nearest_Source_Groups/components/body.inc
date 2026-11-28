#ifndef SIRKIT_TWO_NEAREST_SOURCE_GROUPS_DEFINED
#define SIRKIT_TWO_NEAREST_SOURCE_GROUPS_DEFINED

struct GroupedShortestPathSource
{
    int vertex;
    int group;
};

struct SourceGroupDistance
{
    std::uint64_t distance;
    int source;
    int group;
};

namespace detail
{
template <bool Unit>
auto search_two_source_groups(const ShortestPathAdjacency &graph, std::span<const GroupedShortestPathSource> sources)
{
    struct State
    {
        std::uint64_t distance;
        int vertex;
        int source;
        int group;
    };
    struct Farther
    {
        bool operator()(const State &a, const State &b) const noexcept
        {
            return a.distance > b.distance;
        }
    };
    using Queue = std::conditional_t<Unit, std::queue<State>, std::priority_queue<State, std::vector<State>, Farther>>;
    Queue pending;
    for (const auto &[vertex, group] : sources)
    {
        pending.emplace(0, vertex, vertex, group);
    }

    std::vector<std::array<std::optional<SourceGroupDistance>, 2>> result(graph.size());
    while (!pending.empty())
    {
        const auto [distance, vertex, source, group] = [&]() -> State
        {
            if constexpr (Unit)
            {
                return pending.front();
            }
            else
            {
                return pending.top();
            }
        }();
        pending.pop();
        auto &[first, second] = result[vertex];
        if (second || (first && first->group == group))
        {
            continue;
        }
        (first ? second : first) = SourceGroupDistance{distance, source, group};
        for (const auto &[weight, next] : graph[vertex])
        {
            if constexpr (!Unit)
            {
                if (weight > std::numeric_limits<std::uint64_t>::max() - distance)
                {
                    continue;
                }
            }
            pending.emplace(distance + weight, next, source, group);
        }
    }
    return result;
}
} // namespace detail

inline auto two_nearest_source_groups(
    int n, std::span<const ShortestPathEdge> edges, std::span<const GroupedShortestPathSource> sources,
    bool undirected = false)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::length_error("two_nearest_source_groups: negative vertex count");
    }
    for (const auto &source : sources)
    {
        if (source.vertex < 0 || source.vertex >= n)
        {
            throw std::out_of_range("two_nearest_source_groups: invalid source vertex");
        }
    }
#endif
    detail::ShortestPathAdjacency graph(n);
    bool unit = true;
    for (const auto &[from, to, weight] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("two_nearest_source_groups: invalid edge endpoint");
        }
#endif
        graph[from].emplace_back(weight, to);
        if (undirected)
        {
            graph[to].emplace_back(weight, from);
        }
        unit = unit && weight == 1;
    }
    return unit ? detail::search_two_source_groups<true>(graph, sources)
                : detail::search_two_source_groups<false>(graph, sources);
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
