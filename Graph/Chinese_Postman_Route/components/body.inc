#ifndef SIRKIT_CHINESE_POSTMAN_ROUTE_DEFINED
#define SIRKIT_CHINESE_POSTMAN_ROUTE_DEFINED

struct PostmanEdge
{
    int from, to;
    std::uint64_t weight;
};

struct PostmanRoute
{
    std::uint64_t cost = 0;
    std::vector<int> vertices;
    std::vector<std::size_t> edge_ids;
};

namespace detail
{
inline int other_postman_endpoint(const PostmanEdge &edge, int vertex)
{
    return edge.from == vertex ? edge.to : edge.from;
}

inline std::uint64_t checked_postman_sum(std::uint64_t a, std::uint64_t b)
{
    if (b > std::numeric_limits<std::uint64_t>::max() - a)
    {
        throw std::overflow_error("minimum_postman_route: total cost overflow");
    }
    return a + b;
}

using PostmanAdjacency = std::vector<std::vector<std::size_t>>;

inline std::pair<std::vector<std::optional<std::uint64_t>>, std::vector<std::size_t>> postman_distances(
    const PostmanAdjacency &graph, std::span<const PostmanEdge> edges, int source)
{
    using Entry = std::pair<std::uint64_t, int>;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pending;
    std::vector<std::optional<std::uint64_t>> distance(graph.size());
    std::vector<std::size_t> parent_edge(graph.size());
    distance[source] = 0;
    pending.emplace(0, source);
    while (!pending.empty())
    {
        const auto [prefix, vertex] = pending.top();
        pending.pop();
        if (prefix != distance[vertex].value())
        {
            continue;
        }
        for (const std::size_t id : graph[vertex])
        {
            const auto &edge = edges[id];
            if (edge.weight > std::numeric_limits<std::uint64_t>::max() - prefix)
            {
                continue;
            }
            const int next = other_postman_endpoint(edge, vertex);
            const std::uint64_t next_distance = prefix + edge.weight;
            if (!distance[next] || next_distance < distance[next].value())
            {
                distance[next] = next_distance;
                parent_edge[next] = id;
                pending.emplace(next_distance, next);
            }
        }
    }
    return {std::move(distance), std::move(parent_edge)};
}

inline std::vector<bool> postman_repetitions(
    const PostmanAdjacency &graph, std::span<const PostmanEdge> edges, const std::vector<int> &odd)
{
    std::vector<bool> repeat(edges.size());
    if (odd.empty())
    {
        return repeat;
    }
    const std::size_t k = odd.size();
    std::vector predecessor(k - 1, std::vector<std::size_t>());
    std::vector<std::uint64_t> metric(k * k);
    for (std::size_t from = 0; from + 1 < k; ++from)
    {
        auto [distance, parent_edge] = postman_distances(graph, edges, odd[from]);
        for (std::size_t to = from + 1; to < k; ++to)
        {
            metric[from * k + to] = distance[odd[to]].value();
        }
        predecessor[from] = std::move(parent_edge);
    }

    const auto all = (std::uint32_t{1} << k) - 1;
    std::vector<std::uint64_t> best(all + 1);
    std::vector<std::uint8_t> partner(all + 1);
    const auto solve = [&](auto &&self, std::uint32_t mask) -> std::uint64_t
    {
        if (mask == 0 || partner[mask] != 0)
        {
            return best[mask];
        }
        const auto first = std::countr_zero(mask);
        const std::uint32_t others = mask & (mask - 1);
        for (std::uint32_t available = others; available != 0; available &= available - 1)
        {
            const auto second = std::countr_zero(available);
            const auto suffix = self(self, others ^ (std::uint32_t{1} << second));
            const auto length = metric[first * k + second];
            if (length <= std::numeric_limits<std::uint64_t>::max() - suffix)
            {
                const auto candidate = length + suffix;
                if (partner[mask] == 0 || candidate < best[mask])
                {
                    best[mask] = candidate;
                    partner[mask] = static_cast<std::uint8_t>(second + 1);
                }
            }
        }
        return best[mask];
    };

    (void)solve(solve, all);
    for (auto remaining = all; remaining != 0;)
    {
        const auto u = std::countr_zero(remaining);
        const auto v = partner[remaining] - 1;
        // 両方の最短路に現れる辺を2本ずつ除いても、追加辺の次数の偶奇は維持される。
        for (int current = odd[v]; current != odd[u];)
        {
            const auto edge = predecessor[u][current];
            repeat[edge] = !repeat[edge];
            current = other_postman_endpoint(edges[edge], current);
        }
        remaining &= remaining - 1;
        remaining ^= std::uint32_t{1} << v;
    }
    return repeat;
}
} // namespace detail

[[nodiscard]] inline std::optional<PostmanRoute> minimum_postman_route(
    int n, std::span<const PostmanEdge> edges, int start = -1, int finish = -1)
{
    if (n < 0)
    {
        throw std::length_error("minimum_postman_route: negative vertex count");
    }
    if (start < -1 || start >= n || finish < -1 || finish >= n)
    {
        throw std::out_of_range("minimum_postman_route: terminal");
    }
    if (edges.empty())
    {
        if (n == 0)
        {
            return PostmanRoute{};
        }
        const int first = start == -1 ? 0 : start;
        if (finish != -1 && finish != first)
        {
            return std::nullopt;
        }
        return PostmanRoute{0, {first}, {}};
    }

    detail::PostmanAdjacency graph(n);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto [u, v, weight] = edges[i];
        if (static_cast<std::size_t>(u) >= graph.size() || static_cast<std::size_t>(v) >= graph.size())
        {
            throw std::out_of_range("minimum_postman_route: edge endpoint");
        }
        graph[u].push_back(i);
        graph[v].push_back(i);
    }
    if (start == -1)
    {
        start = edges.front().from;
    }
    if (finish == -1)
    {
        finish = start;
    }

    std::vector<bool> seen(n);
    std::vector<int> pending{start};
    seen[start] = true;
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const int vertex = pending[i];
        for (const auto edge : graph[vertex])
        {
            const int next = detail::other_postman_endpoint(edges[edge], vertex);
            if (!seen[next])
            {
                seen[next] = true;
                pending.push_back(next);
            }
        }
    }
    if (!seen[finish] || std::ranges::any_of(edges, [&](const PostmanEdge &edge)
                                             {
                                                 return !seen[edge.from];
                                             }))
    {
        return std::nullopt;
    }

    std::vector<int> odd;
    for (int vertex = 0; vertex < n; ++vertex)
    {
        const bool required_odd = (vertex == start) != (vertex == finish);
        if ((graph[vertex].size() % 2 != 0) != required_odd)
        {
            odd.push_back(vertex);
        }
    }
    constexpr std::size_t kMaximumOddVertices = 22;
    if (odd.size() > kMaximumOddVertices)
    {
        throw std::length_error("minimum_postman_route: too many odd vertices");
    }
    std::uint64_t cost = 0;
    for (const auto &edge : edges)
    {
        cost = detail::checked_postman_sum(cost, edge.weight);
    }
    const auto repeat = detail::postman_repetitions(graph, edges, odd);

    UndirectedEulerianTrail trail(n);
    trail.reserve_edges(edges.size());
    std::vector<std::size_t> original;
    original.reserve(edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        trail.add_edge(edges[i].from, edges[i].to);
        original.push_back(i);
        if (repeat[i])
        {
            trail.add_edge(edges[i].from, edges[i].to);
            original.push_back(i);
            cost = detail::checked_postman_sum(cost, edges[i].weight);
        }
    }
    auto route = trail.find(start, start == finish).value();
    for (auto &i : route.edges)
    {
        i = original[i];
    }
    return PostmanRoute{cost, std::move(route.vertices), std::move(route.edges)};
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
