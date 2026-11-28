#ifndef SIRKIT_DYNAMIC_STEINER_SUBTREE_DEFINED
#define SIRKIT_DYNAMIC_STEINER_SUBTREE_DEFINED

class DynamicSteinerSubtree
{
public:
    DynamicSteinerSubtree(int n, std::span<const std::pair<int, int>> edges, int root = 0)
        : layout_(n, edges, root), base_(std::bit_ceil(std::max(std::size_t{1}, layout_.size()))), tree_(2 * base_)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (layout_.component_count() > 1)
        {
            throw std::invalid_argument("DynamicSteinerSubtree: expected a connected tree");
        }
#endif
    }

    DynamicSteinerSubtree(const DynamicSteinerSubtree &) = default;
    DynamicSteinerSubtree(DynamicSteinerSubtree &&) noexcept = default;

    DynamicSteinerSubtree &operator=(DynamicSteinerSubtree other) noexcept
    {
        swap_members(other);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return layout_.size();
    }

    [[nodiscard]] bool marked(int v) const
    {
        return tree_[base_ + layout_.order_index(v)].first != -1;
    }

    void set_marked(int v, bool marked = true)
    {
        const auto leaf = base_ + layout_.order_index(v);
        if ((tree_[leaf].first != -1) != marked)
        {
            write_mark(leaf, marked ? v : -1);
        }
    }

    bool toggle(int v)
    {
        const auto leaf = base_ + layout_.order_index(v);
        const bool activated = tree_[leaf].first == -1;
        write_mark(leaf, activated ? v : -1);
        return activated;
    }

    [[nodiscard]] int steiner_vertices() const
    {
        return layout_.empty() ? 0 : count(tree_[1]);
    }

    [[nodiscard]] int steiner_vertices(int v, int root) const
    {
        const auto [left, right] = layout_.subtree_range(v);
        const int root_index = layout_.order_index(root);
        if (v == root)
        {
            return steiner_vertices();
        }
        if (root_index < left || right <= root_index)
        {
            return count(query(left, right));
        }
        const int distance = layout_.depth_of(root) - layout_.depth_of(v);
        const int excluded = layout_.kth_ancestor(root, static_cast<std::uint64_t>(distance - 1));
        const auto [removed_left, removed_right] = layout_.subtree_range(excluded);
        return count(join(query(0, removed_left), query(removed_right, size())));
    }

private:
    struct Summary
    {
        int first = -1;
        int last = -1;
        std::int64_t length = 0;
    };

    void swap_members(DynamicSteinerSubtree &other) noexcept
    {
        layout_.swap(other.layout_);
        std::swap(base_, other.base_);
        tree_.swap(other.tree_);
    }

    Summary join(Summary a, Summary b) const
    {
        if (a.first == -1)
        {
            return b;
        }
        if (b.first == -1)
        {
            return a;
        }
        return {a.first, b.last, a.length + b.length + layout_.get_distance(a.last, b.first)};
    }

    int count(Summary summary) const
    {
        if (summary.first == -1)
        {
            return 0;
        }
        // DFS順の頂点を巡って出発点へ戻ると、部分木の各辺を2回通る。
        return static_cast<int>((summary.length + layout_.get_distance(summary.last, summary.first)) / 2 + 1);
    }

    void write_mark(std::size_t node, int v)
    {
        tree_[node] = {v, v, 0};
        while (node > 1)
        {
            node /= 2;
            tree_[node] = join(tree_[2 * node], tree_[2 * node + 1]);
        }
    }

    Summary query(std::size_t left, std::size_t right) const
    {
        Summary prefix, suffix;
        for (left += base_, right += base_; left < right; left /= 2, right /= 2)
        {
            if (left & 1)
            {
                prefix = join(prefix, tree_[left++]);
            }
            if (right & 1)
            {
                suffix = join(tree_[--right], suffix);
            }
        }
        return join(prefix, suffix);
    }

    LCA layout_;
    std::size_t base_;
    std::vector<Summary> tree_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
