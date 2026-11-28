#ifndef SIRKIT_TREE_MO_DEFINED
#define SIRKIT_TREE_MO_DEFINED

class TreeMo
{
public:
    TreeMo(int n, std::span<const std::pair<int, int>> edges, int root = 0)
        : lca_(checked_size(n), edges, root), entry_(n), edge_to_parent_(n)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (lca_.component_count() > 1)
        {
            throw std::invalid_argument("TreeMo: expected a connected tree");
        }
#endif
        if (n == 0)
        {
            return;
        }
        std::vector<std::vector<int>> children(n);
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            auto [parent, child] = edges[i];
            if (lca_.parent(child) != parent)
            {
                std::swap(parent, child);
            }
            edge_to_parent_[child] = i;
            children[parent].push_back(child);
        }
        struct Frame
        {
            int vertex;
            std::size_t next;
        };
        std::vector<Frame> stack;
        stack.reserve(n);
        tour_.reserve(2 * static_cast<std::size_t>(n));
        visible_tour_size_ = 1;
        tour_.push_back(root);
        stack.push_back({root, 0});
        while (!stack.empty())
        {
            auto &frame = stack.back();
            const int vertex = frame.vertex;
            if (frame.next == children[vertex].size())
            {
                tour_.push_back(vertex);
                stack.pop_back();
            }
            else
            {
                const int child = children[vertex][frame.next++];
                entry_[child] = static_cast<std::int32_t>(tour_.size());
                visible_tour_size_ = entry_[child] + 1;
                tour_.push_back(child);
                stack.push_back({child, 0});
            }
        }
    }

    TreeMo(const TreeMo &) = default;
    TreeMo(TreeMo &&) noexcept = default;

    TreeMo &operator=(TreeMo other) noexcept
    {
        lca_.swap(other.lca_);
        entry_.swap(other.entry_);
        edge_to_parent_.swap(other.edge_to_parent_);
        tour_.swap(other.tour_);
        std::swap(visible_tour_size_, other.visible_tour_size_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return lca_.size();
    }

    template<class Add, class Erase, class Answer>
    void run(std::span<const std::pair<int, int>> queries, Add &&add, Erase &&erase,
             Answer &&answer, std::int32_t block_size = 0) const
    {
        run_impl<false>(queries, add, erase, answer, block_size);
    }

    template<class Add, class Erase, class Answer>
    void run_edges(std::span<const std::pair<int, int>> queries, Add &&add, Erase &&erase,
                   Answer &&answer, std::int32_t block_size = 0) const
    {
        run_impl<true>(queries, add, erase, answer, block_size);
    }

private:
    static int checked_size(int n)
    {
        if (n < 0 || n > std::numeric_limits<std::int32_t>::max() / 2)
        {
            throw std::length_error("TreeMo: tour length out of range");
        }
        return n;
    }

    template<bool Edges, class Add, class Erase, class Answer>
    void run_impl(std::span<const std::pair<int, int>> queries, Add &add, Erase &erase,
                  Answer &answer, std::int32_t block_size) const
    {
        Mo windows(visible_tour_size_);
        windows.reserve_queries(queries.size());
        std::vector<int> correction;
        if constexpr (!Edges)
        {
            correction.reserve(queries.size());
        }
        for (const auto &[u, v] : queries)
        {
            // 頂点の出現回数を奇数で判定すると、2つの根からの経路の対称差が得られる。
            if constexpr (!Edges)
            {
                correction.push_back(lca_.get(u, v));
            }
            else
            {
#ifndef SKIP_RUNTIME_CHECKS
                static_cast<void>(lca_.get(u, v));
#endif
            }
            const auto [left, right] = std::minmax(entry_[u], entry_[v]);
            windows.add_query(left + 1, right + 1);
        }
        std::vector<std::uint8_t> active(size());
        const auto id = [&](int v)
        {
            if constexpr (Edges)
            {
                return edge_to_parent_[v];
            }
            else
            {
                return v;
            }
        };
        const auto toggle = [&](std::int32_t index)
        {
            const int vertex = tour_[index];
            auto &present = active[vertex];
            present ^= 1;
            if (present)
            {
                std::invoke(add, id(vertex));
            }
            else
            {
                std::invoke(erase, id(vertex));
            }
        };
        const auto visit = [&](std::size_t query)
        {
            if constexpr (!Edges)
            {
                std::invoke(add, correction[query]);
            }
            std::invoke(answer, query);
            if constexpr (!Edges)
            {
                std::invoke(erase, correction[query]);
            }
        };
        windows.run(toggle, toggle, visit, block_size);
        for (std::size_t v = 0; v < active.size(); ++v)
        {
            if (active[v])
            {
                std::invoke(erase, id(static_cast<int>(v)));
            }
        }
    }

    LCA lca_;
    std::vector<std::int32_t> entry_;
    std::vector<std::size_t> edge_to_parent_;
    std::vector<int> tour_;
    std::int32_t visible_tour_size_ = 0;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
