#ifndef SIRKIT_INCIDENT_LABEL_SPANNING_TREE_DEFINED
#define SIRKIT_INCIDENT_LABEL_SPANNING_TREE_DEFINED

template <class Label>
struct LabeledSpanningEdge
{
    int from, to;
    Label label;
};

template <class Label>
[[nodiscard]] std::optional<std::vector<std::size_t>> incident_label_spanning_tree(
    std::span<const Label> required, std::span<const LabeledSpanningEdge<Label>> edges)
{
    UnionFind forest(required.size());
    const auto n = static_cast<int>(forest.size());
    std::vector<std::size_t> selected;
    std::vector<bool> covered(n);
    struct Candidate
    {
        int endpoint;
        std::size_t edge;
    };
    std::vector<std::vector<Candidate>> choices(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[u, v, label] = edges[id];
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("incident_label_spanning_tree: edge endpoint");
        }
        const bool covers_u = label == required[u], covers_v = label == required[v];
        if (covers_u && covers_v && forest.merge(u, v))
        {
            selected.push_back(id);
            covered[u] = covered[v] = true;
        }
        if (covers_u)
        {
            choices[v].push_back({u, id});
        }
        if (covers_v)
        {
            choices[u].push_back({v, id});
        }
    }

    std::vector<int> pending;
    for (int v = 0; v < n; ++v)
    {
        if (covered[v])
        {
            pending.push_back(v);
        }
    }
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        for (const auto &[endpoint, id] : choices[pending[i]])
        {
            if (!covered[endpoint])
            {
                covered[endpoint] = true;
                pending.push_back(endpoint);
                selected.push_back(id);
                forest.unite(edges[id].from, edges[id].to);
            }
        }
    }
    if (pending.size() != required.size())
    {
        return std::nullopt;
    }

    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (forest.merge(edges[i].from, edges[i].to))
        {
            selected.push_back(i);
        }
    }
    if (forest.component_count() > 1)
    {
        return std::nullopt;
    }
    return selected;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
