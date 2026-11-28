#ifndef SIRKIT_DYNAMIC_TREE_DISTANCES_DEFINED
#define SIRKIT_DYNAMIC_TREE_DISTANCES_DEFINED

template <class T = std::int64_t>
struct DynamicTreeDistanceEdge
{
    int from, to;
    T weight;
};

template <std::integral T = std::int64_t>
    requires(!std::same_as<T, bool> && sizeof(T) <= sizeof(std::uint64_t))
class DynamicTreeDistances
{
public:
    using Edge = DynamicTreeDistanceEdge<T>;

    DynamicTreeDistances(int n, std::span<const Edge> edges, int root = 0)
    {
        std::vector<std::pair<int, int>> endpoints;
        endpoints.reserve(edges.size());
        for (const auto &[u, v, weight] : edges)
        {
            endpoints.emplace_back(u, v);
        }
        lca_.build(n, endpoints, root);
        std::vector<std::uint64_t> initial(n);
        ranges_.reserve(edges.size());
        weights_.reserve(edges.size());
        for (const auto &[u, v, weight] : edges)
        {
            const int child = lca_.parent(u) == v ? u : v;
            const auto [first, last] = lca_.subtree_range(child);
            initial[first] = static_cast<std::uint64_t>(weight);
            ranges_.emplace_back(first, last);
            weights_.push_back(weight);
        }
        // DFS順では親の時刻が先なので、親の累積値から根距離を作れる。
        std::vector<int> parent_position(n, -1);
        for (int v = 0; v < n; ++v)
        {
            if (const int p = lca_.parent(v); p != -1)
            {
                parent_position[lca_.order_index(v)] = lca_.order_index(p);
            }
        }
        for (int i = 0; i < n; ++i)
        {
            if (const int p = parent_position[i]; p != -1)
            {
                initial[i] += initial[p];
            }
        }
        for (int i = n - 1; i > 0; --i)
        {
            initial[i] -= initial[i - 1];
        }
        differences_ = fenwick_tree<std::uint64_t>(initial);
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return lca_.size();
    }

    [[nodiscard]] std::size_t edge_count() const noexcept
    {
        return weights_.size();
    }

    [[nodiscard]] T weight(std::size_t id) const
    {
        check_edge(id);
        return weights_[id];
    }

    void set_weight(std::size_t id, T weight)
    {
        check_edge(id);
        const auto delta = static_cast<std::uint64_t>(weight) - static_cast<std::uint64_t>(weights_[id]);
        const auto [first, last] = ranges_[id];
        differences_.add(first, delta);
        if (last < differences_.size())
        {
            differences_.add(last, std::uint64_t{0} - delta);
        }
        weights_[id] = weight;
    }

    [[nodiscard]] T root_distance(int vertex)
    {
        return static_cast<T>(root_sum(vertex));
    }

    [[nodiscard]] std::optional<T> distance(int u, int v)
    {
        const int common = lca_.get(u, v);
        if (common == -1)
        {
            return std::nullopt;
        }
        return static_cast<T>(root_sum(u) + root_sum(v) - 2 * root_sum(common));
    }

private:
    [[nodiscard]] std::uint64_t root_sum(int vertex)
    {
        return differences_.sum(0, lca_.order_index(vertex) + 1);
    }

    void check_edge([[maybe_unused]] std::size_t edge) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (edge >= edge_count())
        {
            throw std::out_of_range("dynamic tree distances: edge out of range");
        }
#endif
    }

    LCA lca_;
    fenwick_tree<std::uint64_t> differences_;
    std::vector<T> weights_;
    std::vector<std::pair<int, int>> ranges_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
