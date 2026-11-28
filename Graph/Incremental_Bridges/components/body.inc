#ifndef SIRKIT_INCREMENTAL_BRIDGES_DEFINED
#define SIRKIT_INCREMENTAL_BRIDGES_DEFINED

class IncrementalBridges
{
public:
    explicit IncrementalBridges(std::size_t n)
        : connected_(n), blocks_(n), parent_(n, -1), parent_edge_(n), visited_(n)
    {
    }

    int size() const noexcept
    {
        return connected_.size();
    }

    std::size_t edge_count() const noexcept
    {
        return bridge_flags_.size();
    }

    int bridge_count() const noexcept
    {
        return block_count() - component_count();
    }

    int component_count() const noexcept
    {
        return connected_.component_count();
    }

    int block_count() const noexcept
    {
        return blocks_.component_count();
    }

    bool same_component(int u, int v)
    {
        return connected_.same(u, v);
    }

    bool same_two_edge_component(int u, int v)
    {
        return blocks_.same(u, v);
    }

    void reserve_edges(std::size_t count)
    {
        bridge_flags_.reserve(count);
    }

    bool is_bridge(std::size_t id) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= edge_count())
        {
            throw std::out_of_range("IncrementalBridges: invalid edge ID");
        }
#endif
        return bridge_flags_[id] != 0;
    }

    std::size_t add_edge(int u, int v)
    {
        u = blocks_.leader(u);
        v = blocks_.leader(v);
        const auto id = edge_count();
        if (u == v)
        {
            bridge_flags_.push_back(0);
            return id;
        }
        if (connected_.same(u, v))
        {
            for (auto &path : climbs_)
            {
                path.reserve(static_cast<std::size_t>(size()));
            }
            bridge_flags_.push_back(0);
            merge_cycle(u, v, id + 1);
            return id;
        }
        bridge_flags_.push_back(1);
        if (connected_.size(u) > connected_.size(v))
        {
            std::swap(u, v);
        }
        reroot_smaller_component(u);
        parent_[u] = v;
        parent_edge_[u] = id;
        connected_.merge(u, v);
        return id;
    }

private:
    int forest_parent(int node)
    {
        const int parent = parent_[node];
        return parent == -1 ? -1 : blocks_.leader(parent);
    }

    void reroot_smaller_component(int node)
    {
        int previous = -1;
        std::size_t previous_edge = 0;
        while (node != -1)
        {
            const int next = forest_parent(node);
            parent_[node] = previous;
            std::swap(parent_edge_[node], previous_edge);
            previous = node;
            node = next;
        }
    }

    void merge_cycle(int u, int v, std::size_t stamp)
    {
        climbs_[0].clear();
        climbs_[1].clear();
        int ancestor = -1;
        auto advance = [&](int &node, std::vector<int> &chain)
        {
            if (node == -1)
            {
                return;
            }
            if (visited_[node] == stamp)
            {
                ancestor = node;
                return;
            }
            visited_[node] = stamp;
            chain.push_back(node);
            node = forest_parent(node);
        };
        while (ancestor == -1)
        {
            advance(u, climbs_[0]);
            if (ancestor == -1)
            {
                advance(v, climbs_[1]);
            }
        }

        // 共通祖先の外側につながる橋を、併合後の代表へ引き継ぐ。
        const int outside = parent_[ancestor];
        const auto outside_edge = parent_edge_[ancestor];
        for (const auto &chain : climbs_)
        {
            for (const int node : chain)
            {
                if (node == ancestor)
                {
                    break;
                }
                bridge_flags_[parent_edge_[node]] = 0;
                blocks_.merge(ancestor, node);
            }
        }
        const int leader = blocks_.leader(ancestor);
        parent_[leader] = outside;
        parent_edge_[leader] = outside_edge;
    }

    UnionFind connected_;
    UnionFind blocks_;
    std::vector<int> parent_;
    std::vector<std::size_t> parent_edge_;
    std::vector<std::size_t> visited_;
    std::vector<unsigned char> bridge_flags_;
    std::array<std::vector<int>, 2> climbs_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
