#ifndef SIRKIT_TREE_DISTANCE_SUMS_DEFINED
#define SIRKIT_TREE_DISTANCE_SUMS_DEFINED

template <typename T>
struct TreeDistanceSumEdge
{
    int from;
    int to;
    T length = T{1};
};

template <typename T = std::uint64_t>
class TreeDistanceSums
{
    using Number = std::conditional_t<std::is_integral_v<T>, std::uint64_t, T>;
    static_assert(!std::is_integral_v<T> || (sizeof(T) <= sizeof(std::uint64_t) && !std::is_same_v<T, bool>));

    struct Moments
    {
        Number weighted_length{};
        Number weighted_depth{};
        Number length{};
        Number depth_length{};
    };

    static Moments combine(const Moments &a, const Moments &b)
    {
        return {a.weighted_length + b.weighted_length, a.weighted_depth + b.weighted_depth,
                a.length + b.length, a.depth_length + b.depth_length};
    }

    static Moments identity()
    {
        return {};
    }

    static Moments update(const Number &delta, Moments value)
    {
        value.weighted_length = value.weighted_length + delta * value.length;
        value.weighted_depth = value.weighted_depth + delta * value.depth_length;
        return value;
    }

    static Number compose(const Number &a, const Number &b)
    {
        return a + b;
    }

    static Number zero()
    {
        return {};
    }

    using Aggregates = lazy_segtree<Moments, combine, identity, Number, update, compose, zero>;

public:
    TreeDistanceSums(int n, std::span<const TreeDistanceSumEdge<T>> edges,
                     std::span<const T> weights = {}, int root = 0)
    {
        if (n < 0)
        {
            throw std::length_error("TreeDistanceSums: negative vertex count");
        }
        if (edges.size() != static_cast<std::size_t>(n == 0 ? 0 : n - 1) ||
            (!weights.empty() && weights.size() != static_cast<std::size_t>(n)))
        {
            throw std::invalid_argument("TreeDistanceSums: inconsistent sizes");
        }
        Graph graph(n);
        for (const auto &[u, v, length] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (static_cast<std::size_t>(u) >= graph.size() || static_cast<std::size_t>(v) >= graph.size())
            {
                throw std::out_of_range("TreeDistanceSums: invalid vertex");
            }
#endif
            graph[u].push_back(v);
            graph[v].push_back(u);
        }
        layout_.build(graph, root);
        std::vector<Number> edge_length(n), subtree_weight(n);
        for (const auto &[u, v, length] : edges)
        {
            const int child = layout_.parent(u) == v ? u : v;
            edge_length[child] = static_cast<Number>(length);
        }
        depth_length_.resize(n);
        prefix_length_.resize(n);
        for (const int v : layout_.order())
        {
            const int parent = layout_.parent(v);
            if (parent != -1)
            {
                depth_length_[v] = depth_length_[parent] + edge_length[v];
                prefix_length_[v] = prefix_length_[parent] + depth_length_[v];
            }
            if (!weights.empty())
            {
                subtree_weight[v] = static_cast<Number>(weights[v]);
                total_weight_ = total_weight_ + subtree_weight[v];
                weighted_origin_ = weighted_origin_ + subtree_weight[v] * depth_length_[v];
            }
        }
        std::vector<Moments> leaves(n);
        for (const int v : std::views::reverse(layout_.order()))
        {
            const int parent = layout_.parent(v);
            if (parent != -1)
            {
                subtree_weight[parent] = subtree_weight[parent] + subtree_weight[v];
            }
            const auto length = edge_length[v];
            const auto depth = static_cast<Number>(layout_.depth(v));
            const auto weighted = length * subtree_weight[v];
            leaves[layout_.index(v)] = {weighted, weighted * depth, length, length * depth};
        }
        aggregates_.assign(leaves);
    }

    void add(int v, const T &delta)
    {
        const auto amount = static_cast<Number>(delta);
        layout_.path_apply(layout_.root(), v, aggregates_, amount, true);
        total_weight_ = total_weight_ + amount;
        weighted_origin_ = weighted_origin_ + amount * depth_length_[v];
    }

    [[nodiscard]] T sum_at(int v)
    {
        const auto moment = origin_moments(v);
        return static_cast<T>(vertex_value(v, moment));
    }

    [[nodiscard]] T sum_path(int u, int v)
    {
        const int middle = layout_.lca(u, v);
        const auto middle_moments = origin_moments(middle);
        const auto middle_prefix = prefix_value(middle, middle_moments);
        const auto first = u == middle ? middle_prefix : prefix_value(u, origin_moments(u));
        const auto last = v == middle ? middle_prefix : prefix_value(v, origin_moments(v));
        return static_cast<T>(first + last - middle_prefix - middle_prefix + vertex_value(middle, middle_moments));
    }

private:
    [[nodiscard]] Moments origin_moments(int v)
    {
        return layout_.path_prod(layout_.root(), v, Moments{}, aggregates_, combine, true);
    }

    [[nodiscard]] Number vertex_value(int v, const Moments &value) const
    {
        return total_weight_ * depth_length_[v] + weighted_origin_ - value.weighted_length - value.weighted_length;
    }

    [[nodiscard]] Number prefix_value(int v, const Moments &value) const
    {
        const auto count = static_cast<Number>(static_cast<std::uint64_t>(layout_.depth(v)) + 1);
        const auto lca_sum = count * value.weighted_length - value.weighted_depth;
        return total_weight_ * prefix_length_[v] + weighted_origin_ * count - lca_sum - lca_sum;
    }

    HeavyLightDecomposition layout_;
    Aggregates aggregates_;
    std::vector<Number> depth_length_;
    std::vector<Number> prefix_length_;
    Number total_weight_{};
    Number weighted_origin_{};
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
