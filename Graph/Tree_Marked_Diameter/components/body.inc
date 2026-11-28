#ifndef SIRKIT_TREE_MARKED_DIAMETER_DEFINED
#define SIRKIT_TREE_MARKED_DIAMETER_DEFINED

class TreeMarkedDiameter
{
public:
    struct Diameter
    {
        int first;
        int second;
        int distance;
    };

    struct Farthest
    {
        int vertex;
        int distance;
    };

    TreeMarkedDiameter(int n, std::span<const std::pair<int, int>> edges, bool initially_marked = false)
        : layout_(n, edges), base_(std::bit_ceil(std::max(std::size_t{1}, layout_.size()))), tree_(2 * base_, kEmpty)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (layout_.component_count() > 1)
        {
            throw std::invalid_argument("TreeMarkedDiameter: expected a connected tree");
        }
#endif
        if (initially_marked)
        {
            for (int v = 0; v < n; ++v)
            {
                tree_[leaf(v)] = {v, v, 0};
            }
            for (std::size_t node = base_ - 1; node > 0; --node)
            {
                refresh(node);
            }
        }
    }

    TreeMarkedDiameter(const TreeMarkedDiameter &) = default;
    TreeMarkedDiameter(TreeMarkedDiameter &&) noexcept = default;

    TreeMarkedDiameter &operator=(TreeMarkedDiameter other) noexcept
    {
        layout_.swap(other.layout_);
        std::swap(base_, other.base_);
        tree_.swap(other.tree_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return layout_.size();
    }

    [[nodiscard]] bool marked(int v) const
    {
        return tree_[leaf(v)].first != -1;
    }

    void set_marked(int v, bool marked = true)
    {
        const auto node = leaf(v);
        if ((tree_[node].first != -1) != marked)
        {
            write_mark(node, marked ? v : -1);
        }
    }

    bool toggle(int v)
    {
        const auto node = leaf(v);
        const bool activated = tree_[node].first == -1;
        write_mark(node, activated ? v : -1);
        return activated;
    }

    [[nodiscard]] std::optional<Diameter> diameter() const noexcept
    {
        if (layout_.empty())
        {
            return std::nullopt;
        }
        return to_optional(tree_[1]);
    }

    [[nodiscard]] std::optional<Diameter> subtree_diameter(int v) const
    {
        const auto [first, last] = layout_.subtree_range(v);
        std::size_t left = base_ + static_cast<std::size_t>(first);
        std::size_t right = base_ + static_cast<std::size_t>(last);
        Diameter answer = kEmpty;
        for (; left < right; left /= 2, right /= 2)
        {
            if (left & 1)
            {
                answer = join(answer, tree_[left++]);
            }
            if (right & 1)
            {
                answer = join(answer, tree_[--right]);
            }
        }
        return to_optional(answer);
    }

    [[nodiscard]] std::optional<Farthest> farthest(int v) const
    {
        const auto ends = diameter();
        if (!ends)
        {
            static_cast<void>(layout_.depth_of(v));
            return std::nullopt;
        }
        const int left = layout_.get_distance(v, ends->first);
        const int right = layout_.get_distance(v, ends->second);
        return left >= right ? Farthest{ends->first, left} : Farthest{ends->second, right};
    }

private:
    static constexpr Diameter kEmpty{-1, -1, 0};

    [[nodiscard]] std::size_t leaf(int v) const
    {
        return base_ + static_cast<std::size_t>(layout_.order_index(v));
    }

    static std::optional<Diameter> to_optional(Diameter answer) noexcept
    {
        if (answer.first == -1)
        {
            return std::nullopt;
        }
        return answer;
    }

    [[nodiscard]] Diameter join(Diameter a, Diameter b) const
    {
        if (a.first == -1)
        {
            return b;
        }
        if (b.first == -1)
        {
            return a;
        }
        // 集合内の任意の頂点からの最遠距離は、直径のいずれかの端点までの距離で得られる。
        Diameter best = a.distance >= b.distance ? a : b;
        for (const int u : {a.first, a.second})
        {
            for (const int v : {b.first, b.second})
            {
                const int distance = layout_.get_distance(u, v);
                if (distance > best.distance)
                {
                    best = {u, v, distance};
                }
            }
        }
        return best;
    }

    void refresh(std::size_t node)
    {
        tree_[node] = join(tree_[2 * node], tree_[2 * node + 1]);
    }

    void write_mark(std::size_t node, int vertex)
    {
        tree_[node] = {vertex, vertex, 0};
        while (node > 1)
        {
            node /= 2;
            refresh(node);
        }
    }

    LCA layout_;
    std::size_t base_;
    std::vector<Diameter> tree_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
