#ifndef SIRKIT_NEAR_TREE_DISTANCES_DEFINED
#define SIRKIT_NEAR_TREE_DISTANCES_DEFINED

class NearTreeDistances
{
    using Adjacency = std::vector<std::vector<int>>;

    struct Location
    {
        int component = -1;
        int offset = 0;
    };

public:
    NearTreeDistances(int n, std::span<const std::pair<int, int>> edges)
        : vertices_(checked_size(n)), tree_(prepare(edges))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(vertices_.size());
    }

    [[nodiscard]] int distance(int from, int to) const
    {
        check_vertex(from);
        check_vertex(to);
        const auto a = vertices_[from];
        const auto b = vertices_[to];
        if (a.component != b.component)
        {
            return -1;
        }
        int answer = tree_.distance(from, to);
        for (const auto &distance : from_hub_[a.component])
        {
            const auto through_hub = static_cast<std::int64_t>(distance[a.offset]) + distance[b.offset];
            if (through_hub < answer)
            {
                answer = static_cast<int>(through_hub);
            }
        }
        return answer;
    }

private:
    static int checked_size(int n)
    {
        if (n < 0)
        {
            throw std::length_error("NearTreeDistances: negative size");
        }
        return n;
    }

    void check_vertex(int v) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || v >= size())
        {
            throw std::out_of_range("NearTreeDistances: invalid vertex");
        }
#else
        static_cast<void>(v);
#endif
    }

    std::vector<int> bfs(const Adjacency &graph, int source, std::size_t n, std::vector<int> &pending) const
    {
        std::vector<int> distance(n, -1);
        pending.clear();
        pending.push_back(source);
        distance[vertices_[source].offset] = 0;
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const int u = pending[i];
            const int next_distance = distance[vertices_[u].offset] + 1;
            for (const int v : graph[u])
            {
                int &value = distance[vertices_[v].offset];
                if (value == -1)
                {
                    value = next_distance;
                    pending.push_back(v);
                }
            }
        }
        return distance;
    }

    UndirectedPseudoforest prepare(std::span<const std::pair<int, int>> edges)
    {
        const int n = size();
        Adjacency graph(n);
        for (const auto &[u, v] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            check_vertex(u);
            check_vertex(v);
#endif
            graph[u].push_back(v);
            graph[v].push_back(u);
        }
        std::vector<std::pair<int, int>> tree;
        tree.reserve(n);
        std::vector<int> parent(n, -1);
        std::vector<bool> hub(n);
        std::vector<int> vertices;
        vertices.reserve(n);
        std::vector<int> pending;
        for (int root = 0; root < n; ++root)
        {
            if (vertices_[root].component != -1)
            {
                continue;
            }
            const int component = static_cast<int>(from_hub_.size());
            auto &rows = from_hub_.emplace_back();
            vertices.clear();
            vertices.push_back(root);
            vertices_[root] = {component, 0};
            for (std::size_t i = 0; i < vertices.size(); ++i)
            {
                const int u = vertices[i];
                for (const int v : graph[u])
                {
                    if (vertices_[v].component == -1)
                    {
                        vertices_[v] = {component, static_cast<int>(vertices.size())};
                        tree.emplace_back(u, v);
                        parent[v] = u;
                        vertices.push_back(v);
                    }
                }
            }

            // A hub at either endpoint covers a non-tree edge. Loops and extra copies of a tree edge are redundant.
            for (const int u : vertices)
            {
                for (const int v : graph[u])
                {
                    if (u < v && parent[u] != v && parent[v] != u && !hub[u] && !hub[v])
                    {
                        hub[u] = true;
                        rows.push_back(bfs(graph, u, vertices.size(), pending));
                    }
                }
            }
        }
        return UndirectedPseudoforest::build(n, tree).value();
    }

    std::vector<Location> vertices_;
    std::vector<std::vector<std::vector<int>>> from_hub_;
    UndirectedPseudoforest tree_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
