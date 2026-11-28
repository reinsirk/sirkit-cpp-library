#ifndef SIRKIT_TREE_DISTANCE_LAYERS_DEFINED
#define SIRKIT_TREE_DISTANCE_LAYERS_DEFINED

class TreeDistanceLayers
{
public:
    TreeDistanceLayers(int n, std::span<const std::pair<int, int>> edges) : lca_(n, edges)
    {
        diameter_.assign(lca_.component_count(), {-1, -1});
        for (int v = 0; v < n; ++v)
        {
            auto &first = diameter_[lca_.component_of(v)].first;
            if (first == -1 || lca_.depth_of(v) > lca_.depth_of(first))
            {
                first = v;
            }
        }
        std::vector<int> distance_from_first(n);
        for (int v = 0; v < n; ++v)
        {
            auto &[first, second] = diameter_[lca_.component_of(v)];
            const int distance = lca_.get_distance(first, v);
            distance_from_first[v] = distance;
            if (second == -1 || distance > distance_from_first[second])
            {
                second = v;
            }
        }
        farthest_.resize(n);
        for (int v = 0; v < n; ++v)
        {
            const auto [first, second] = diameter_[lca_.component_of(v)];
            const int distance = lca_.get_distance(second, v);
            farthest_[v] = distance_from_first[v] >= distance
                               ? Farthest{first, distance_from_first[v]}
                               : Farthest{second, distance};
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return lca_.size();
    }

    [[nodiscard]] int farthest(int vertex) const
    {
        return get(vertex).vertex;
    }

    [[nodiscard]] int eccentricity(int vertex) const
    {
        return get(vertex).distance;
    }

    [[nodiscard]] std::pair<int, int> diameter_endpoints(int vertex) const
    {
        return diameter_[lca_.component_of(vertex)];
    }

    [[nodiscard]] int vertex_at_distance(int vertex, int distance) const
    {
        const auto &entry = get(vertex);
        if (distance < 0 || distance > entry.distance)
        {
            return -1;
        }
        return lca_.kth_on_path(vertex, entry.vertex, distance);
    }

private:
    struct Farthest
    {
        int vertex, distance;
    };

    [[nodiscard]] const Farthest &get(int vertex) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || static_cast<std::size_t>(vertex) >= size())
        {
            throw std::out_of_range("tree distance layers: vertex out of range");
        }
#endif
        return farthest_[vertex];
    }

    LCA lca_;
    std::vector<std::pair<int, int>> diameter_;
    std::vector<Farthest> farthest_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
