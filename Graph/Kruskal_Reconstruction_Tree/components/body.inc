#ifndef SIRKIT_KRUSKAL_RECONSTRUCTION_TREE_DEFINED
#define SIRKIT_KRUSKAL_RECONSTRUCTION_TREE_DEFINED

enum class KruskalOrder
{
    increasing,
    decreasing,
};

namespace detail
{
inline int kruskal_reconstruction_vertices(int n)
{
    if (n < 0 || n > std::numeric_limits<int>::max() / 2)
    {
        throw std::length_error("KruskalReconstructionTree: invalid vertex count");
    }
    return n;
}
} // namespace detail

class KruskalReconstructionTree
{
public:
    struct Node
    {
        int parent = -1;
        int left = -1;
        int right = -1;
        int vertex_count = 1;
        std::optional<std::int64_t> weight;
    };

    KruskalReconstructionTree(int n, std::span<const SpanningTreeEdge> edges,
                              KruskalOrder order = KruskalOrder::increasing)
        : n_(detail::kruskal_reconstruction_vertices(n)), order_(order), nodes_(n)
    {
        std::vector<SpanningTreeEdge> inverted;
        std::span<const SpanningTreeEdge> ordered = edges;
        if (order == KruskalOrder::decreasing)
        {
            inverted.assign(edges.begin(), edges.end());
            for (auto &edge : inverted)
            {
                edge.weight = ~edge.weight;
            }
            ordered = inverted;
        }
        const auto forest = minimum_spanning_forest(n, ordered);

        UnionFind sets(n);
        std::vector<int> representative(n);
        std::iota(representative.begin(), representative.end(), 0);
        nodes_.reserve(n + forest.size());
        std::vector<std::pair<int, int>> tree_edges;
        tree_edges.reserve(n + forest.size());
        for (const auto id : forest)
        {
            const auto &[u, v, weight] = edges[id];
            const int a = sets.root(u);
            const int b = sets.root(v);
            const int left = representative[a];
            const int right = representative[b];
            const int node = node_count();
            nodes_[left].parent = nodes_[right].parent = node;
            nodes_.push_back({-1, left, right, nodes_[left].vertex_count + nodes_[right].vertex_count, weight});
            tree_edges.emplace_back(node, left);
            tree_edges.emplace_back(node, right);
            sets.merge(a, b);
            representative[sets.root(a)] = node;
        }

        // The virtual root lets the existing forest LCA use the top of every merge tree.
        const int virtual_root = node_count();
        for (int v = 0; v < virtual_root; ++v)
        {
            if (nodes_[v].parent == -1)
            {
                tree_edges.emplace_back(virtual_root, v);
            }
        }
        ancestor_.build(virtual_root + 1, tree_edges, virtual_root);
    }

    [[nodiscard]] int size() const noexcept
    {
        return n_;
    }

    [[nodiscard]] int node_count() const noexcept
    {
        return static_cast<int>(nodes_.size());
    }

    [[nodiscard]] const Node &node(int index) const
    {
        check_index(index, node_count());
        return nodes_[index];
    }

    [[nodiscard]] int merge_node(int u, int v) const
    {
        check_index(u, n_);
        check_index(v, n_);
        const int at = ancestor_.get(u, v);
        return at == node_count() ? -1 : at;
    }

    [[nodiscard]] std::optional<std::int64_t> merge_weight(int u, int v) const
    {
        const int at = merge_node(u, v);
        return at == -1 ? std::nullopt : nodes_[at].weight;
    }

    [[nodiscard]] int component_at(int v, std::int64_t threshold) const
    {
        check_index(v, n_);
        int first = 0;
        int last = ancestor_.depth_of(v);
        while (last - first > 1)
        {
            const int middle = first + (last - first) / 2;
            const int candidate = ancestor_.kth_ancestor(v, middle);
            const auto weight = nodes_[candidate].weight.value();
            const bool allowed = order_ == KruskalOrder::increasing ? weight <= threshold : weight >= threshold;
            if (allowed)
            {
                first = middle;
            }
            else
            {
                last = middle;
            }
        }
        return ancestor_.kth_ancestor(v, first);
    }

private:
    static void check_index(int index, int limit)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (index < 0 || index >= limit)
        {
            throw std::out_of_range("KruskalReconstructionTree: invalid node");
        }
#else
        static_cast<void>(index);
        static_cast<void>(limit);
#endif
    }

    int n_;
    KruskalOrder order_;
    std::vector<Node> nodes_;
    LCA ancestor_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
