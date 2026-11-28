#ifndef SIRKIT_SUBTREE_EDGE_ADD_DISTANCES_DEFINED
#define SIRKIT_SUBTREE_EDGE_ADD_DISTANCES_DEFINED

template <std::integral T = std::int64_t>
    requires(!std::same_as<T, bool> && sizeof(T) <= sizeof(std::uint64_t))
class SubtreeEdgeAddDistances
{
public:
    struct Edge
    {
        int from;
        int to;
        T weight{};
    };

    SubtreeEdgeAddDistances(int n, std::span<const Edge> edges, int root = 0)
    {
        std::vector<std::pair<int, int>> endpoints;
        endpoints.reserve(edges.size());
        for (const auto &[u, v, weight] : edges)
        {
            endpoints.emplace_back(u, v);
        }
        lca_.build(n, endpoints, root);
        std::vector<Coefficients> initial(n);
        for (const auto &[u, v, weight] : edges)
        {
            const int child = lca_.parent(v) == u ? v : u;
            initial[lca_.order_index(child)].offset = static_cast<std::uint64_t>(weight);
        }
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
                initial[i].offset += initial[p].offset;
            }
        }
        for (int i = n - 1; i > 0; --i)
        {
            initial[i].offset -= initial[i - 1].offset;
        }
        differences_ = fenwick_tree<Coefficients>(initial);
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return lca_.size();
    }

    void add_subtree(int vertex, T delta)
    {
        const auto [first, last] = lca_.subtree_range(vertex);
        const auto slope = static_cast<std::uint64_t>(delta);
        const auto product = slope * static_cast<std::uint64_t>(lca_.depth_of(vertex));
        // 子孫 w の根距離には delta * (depth(w) - depth(vertex)) を足す。
        differences_.add(first, {slope, 0 - product});
        if (last < differences_.size())
        {
            differences_.add(last, {0 - slope, product});
        }
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
    struct Coefficients
    {
        std::uint64_t slope = 0;
        std::uint64_t offset = 0;

        void operator+=(Coefficients other)
        {
            slope += other.slope;
            offset += other.offset;
        }

        friend Coefficients operator-(Coefficients left, Coefficients right)
        {
            return {left.slope - right.slope, left.offset - right.offset};
        }
    };

    [[nodiscard]] std::uint64_t root_sum(int vertex)
    {
        const auto sum = differences_.sum(0, lca_.order_index(vertex) + 1);
        return sum.offset + sum.slope * static_cast<std::uint64_t>(lca_.depth_of(vertex));
    }

    LCA lca_;
    fenwick_tree<Coefficients> differences_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
