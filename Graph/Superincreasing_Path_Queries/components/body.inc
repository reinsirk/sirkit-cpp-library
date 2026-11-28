#ifndef SIRKIT_SUPERINCREASING_PATH_QUERIES_DEFINED
#define SIRKIT_SUPERINCREASING_PATH_QUERIES_DEFINED

template <class T>
struct SuperincreasingPathEdge
{
    int from, to;
    T value;
};

template <class T>
class SuperincreasingPathQueries
{
public:
    SuperincreasingPathQueries(int n, std::span<const SuperincreasingPathEdge<T>> edges)
        : edges_(edges.begin(), edges.end()), child_(edges.size(), -1)
    {
        if (n < 0)
        {
            throw std::length_error("SuperincreasingPathQueries: negative vertex count");
        }
        std::vector<SpanningTreeEdge> ranked;
        ranked.reserve(edges.size());
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto &edge = edges[i];
            if (edge.from < 0 || edge.from >= n || edge.to < 0 || edge.to >= n)
            {
                throw std::out_of_range("SuperincreasingPathQueries: edge endpoint");
            }
            ranked.push_back({edge.from, edge.to, static_cast<std::int64_t>(i)});
        }
        auto sensitivity = minimum_spanning_forest_sensitivity(n, ranked);
        std::vector<std::pair<int, int>> forest;
        forest.reserve(sensitivity.forest.size());
        for (const auto id : sensitivity.forest)
        {
            const auto &edge = edges[id];
            forest.emplace_back(edge.from, edge.to);
        }
        lca_.build(n, forest);

        prefix_.resize(n);
        for (const auto id : sensitivity.forest)
        {
            const auto &edge = edges[id];
            const int child = lca_.parent(edge.from) == edge.to ? edge.from : edge.to;
            child_[id] = child;
            prefix_[child] = edge.value;
        }
        std::vector<int> preorder(n);
        for (int v = 0; v < n; ++v)
        {
            preorder[lca_.order_index(v)] = v;
        }
        for (const int vertex : preorder)
        {
            const int parent = lca_.parent(vertex);
            if (parent != -1)
            {
                prefix_[vertex] = prefix_[parent] + prefix_[vertex];
            }
        }
        exchanges_ = std::move(sensitivity.exchange);
    }

    [[nodiscard]] std::optional<T> path_sum(int from, int to) const
    {
        CheckVertex(from);
        CheckVertex(to);
        const int ancestor = lca_(from, to);
        if (ancestor == -1)
        {
            return std::nullopt;
        }
        return PathSum(from, to, ancestor);
    }

    [[nodiscard]] std::optional<T> path_sum_without_edge(int from, int to, std::size_t removed) const
    {
        if (removed >= edges_.size())
        {
            throw std::out_of_range("SuperincreasingPathQueries: edge id");
        }
        CheckVertex(from);
        CheckVertex(to);
        const int ancestor = lca_(from, to);
        if (ancestor == -1)
        {
            return std::nullopt;
        }
        const int child = child_[removed];
        if (child == -1)
        {
            return PathSum(from, to, ancestor);
        }
        const bool from_subtree = lca_.is_ancestor(child, from);
        if (from_subtree == lca_.is_ancestor(child, to))
        {
            return PathSum(from, to, ancestor);
        }
        if (!exchanges_[removed])
        {
            return std::nullopt;
        }

        const auto &bridge = edges_[*exchanges_[removed]];
        int start = bridge.from, finish = bridge.to;
        if (from_subtree != lca_.is_ancestor(child, start))
        {
            std::swap(start, finish);
        }
        return PathSum(from, start, lca_(from, start)) + bridge.value +
               PathSum(finish, to, lca_(finish, to));
    }

private:
    std::vector<SuperincreasingPathEdge<T>> edges_;
    std::vector<int> child_;
    std::vector<std::optional<std::size_t>> exchanges_;
    LCA lca_;
    std::vector<T> prefix_;

    void CheckVertex(int vertex) const
    {
        if (static_cast<std::size_t>(vertex) >= lca_.size())
        {
            throw std::out_of_range("SuperincreasingPathQueries: vertex");
        }
    }

    T PathSum(int from, int to, int ancestor) const
    {
        return (prefix_[from] - prefix_[ancestor]) + (prefix_[to] - prefix_[ancestor]);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
