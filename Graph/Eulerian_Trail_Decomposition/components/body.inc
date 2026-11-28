#ifndef SIRKIT_EULERIAN_TRAIL_DECOMPOSITION_DEFINED
#define SIRKIT_EULERIAN_TRAIL_DECOMPOSITION_DEFINED

template <bool Directed = false>
std::vector<typename EulerianTrail<Directed>::Trail> minimum_eulerian_trail_decomposition(
    int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0 || n == std::numeric_limits<int>::max())
    {
        throw std::length_error("minimum_eulerian_trail_decomposition: invalid vertex count");
    }
    if (edges.empty())
    {
        return {};
    }

    const int hub = n;
    EulerianTrail<Directed> graph(n + 1);
    graph.reserve_edges(edges.size());
    std::vector<std::size_t> begin(static_cast<std::size_t>(n) + 1);
    std::vector<std::ptrdiff_t> balance;
    if constexpr (Directed)
    {
        balance.resize(n);
    }
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("minimum_eulerian_trail_decomposition: invalid vertex");
        }
#endif
        graph.add_edge(u, v);
        ++begin[u + 1];
        ++begin[v + 1];
        if constexpr (Directed)
        {
            ++balance[u];
            --balance[v];
        }
    }
    for (int u = 0; u < n; ++u)
    {
        begin[u + 1] += begin[u];
    }

    std::vector<int> neighbors(begin.back());
    {
        std::vector<std::size_t> next(begin.begin(), begin.end() - 1);
        for (const auto &[u, v] : edges)
        {
            neighbors[next[u]++] = v;
            neighbors[next[v]++] = u;
        }
    }

    std::vector<unsigned char> visited(n);
    std::vector<int> stack;
    for (int start = 0; start < n; ++start)
    {
        if (visited[start] || begin[start] == begin[start + 1])
        {
            continue;
        }
        bool has_hub_edge = false;
        visited[start] = 1;
        stack.push_back(start);
        while (!stack.empty())
        {
            const int u = stack.back();
            stack.pop_back();
            if constexpr (Directed)
            {
                if (balance[u] != 0)
                {
                    has_hub_edge = true;
                }
                for (; balance[u] > 0; --balance[u])
                {
                    graph.add_edge(hub, u);
                }
                for (; balance[u] < 0; ++balance[u])
                {
                    graph.add_edge(u, hub);
                }
            }
            else
            {
                if ((begin[u + 1] - begin[u]) % 2 != 0)
                {
                    graph.add_edge(hub, u);
                    has_hub_edge = true;
                }
            }
            for (std::size_t i = begin[u]; i < begin[u + 1]; ++i)
            {
                const int v = neighbors[i];
                if (!visited[v])
                {
                    visited[v] = 1;
                    stack.push_back(v);
                }
            }
        }
        if (!has_hub_edge)
        {
            graph.add_edge(hub, start);
            graph.add_edge(start, hub);
        }
    }

    // 仮の頂点を始点とする閉路を取り出し、仮の頂点に戻るたびに小道を区切る。
    const auto circuit = graph.find(hub, true).value();
    std::vector<typename EulerianTrail<Directed>::Trail> trails;
    for (std::size_t i = 0; i < circuit.edges.size(); ++i)
    {
        const int u = circuit.vertices[i];
        const int v = circuit.vertices[i + 1];
        if (u == hub)
        {
            trails.emplace_back();
            trails.back().vertices.push_back(v);
        }
        else if (v != hub)
        {
            trails.back().vertices.push_back(v);
            trails.back().edges.push_back(circuit.edges[i]);
        }
    }
    return trails;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
