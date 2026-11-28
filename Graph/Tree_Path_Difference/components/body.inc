#ifndef SIRKIT_TREE_PATH_DIFFERENCE_DEFINED
#define SIRKIT_TREE_PATH_DIFFERENCE_DEFINED

template <std::integral T = std::int64_t>
    requires(!std::same_as<T, bool> && sizeof(T) <= sizeof(std::uint64_t))
class TreePathDifference
{
public:
    TreePathDifference(int n, std::span<const std::pair<int, int>> edges, int root = 0)
        : lca_(n, edges, root),
          vertices_(n),
          edges_(n),
          child_(edges.size()),
          order_(n)
    {
        for (int v = 0; v < n; ++v)
        {
            order_[lca_.order_index(v)] = v;
        }
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto [u, v] = edges[i];
            child_[i] = lca_.parent(u) == v ? u : v;
        }
    }

    bool add_vertex_path(int u, int v, T delta)
    {
        const int common = lca_.get(u, v);
        if (common == -1)
        {
            return false;
        }
        const auto value = static_cast<std::uint64_t>(delta);
        vertices_[u] += value;
        vertices_[v] += value;
        vertices_[common] -= value;
        if (const int parent = lca_.parent(common); parent != -1)
        {
            vertices_[parent] -= value;
        }
        return true;
    }

    bool add_edge_path(int u, int v, T delta)
    {
        const int common = lca_.get(u, v);
        if (common == -1)
        {
            return false;
        }
        const auto value = static_cast<std::uint64_t>(delta);
        edges_[u] += value;
        edges_[v] += value;
        edges_[common] -= 2 * value;
        return true;
    }

    [[nodiscard]] std::vector<T> vertex_values() const
    {
        const auto totals = accumulate(vertices_);
        return std::vector<T>(totals.begin(), totals.end());
    }

    [[nodiscard]] std::vector<T> edge_values() const
    {
        const auto totals = accumulate(edges_);
        std::vector<T> result;
        result.reserve(child_.size());
        for (int v : child_)
        {
            result.push_back(static_cast<T>(totals[v]));
        }
        return result;
    }

private:
    [[nodiscard]] std::vector<std::uint64_t> accumulate(std::vector<std::uint64_t> difference) const
    {
        for (auto it = order_.rbegin(); it != order_.rend(); ++it)
        {
            const int v = *it;
            if (const int parent = lca_.parent(v); parent != -1)
            {
                difference[parent] += difference[v];
            }
        }
        return difference;
    }

    LCA lca_;
    std::vector<std::uint64_t> vertices_;
    std::vector<std::uint64_t> edges_;
    std::vector<int> child_;
    std::vector<int> order_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
