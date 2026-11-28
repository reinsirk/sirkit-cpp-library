#ifndef SIRKIT_STRONG_CONNECTIVITY_AUGMENTATION_DEFINED
#define SIRKIT_STRONG_CONNECTIVITY_AUGMENTATION_DEFINED

[[nodiscard]] inline std::vector<std::pair<int, int>> strong_connectivity_augmentation(
    int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("strong_connectivity_augmentation: vertex count");
    }
    scc_graph graph(n);
    graph.reserve_edges(edges.size());
    for (const auto &[from, to] : edges)
    {
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("strong_connectivity_augmentation: edge endpoint");
        }
        graph.add_edge(from, to);
    }
    const auto groups = graph.scc();
    if (groups.size() < 2)
    {
        return {};
    }
    const auto dag = graph.condensation(false);
    std::vector<bool> incoming(dag.size()), seen(dag.size());
    for (const auto &outgoing : dag)
    {
        for (const int next : outgoing)
        {
            incoming[next] = true;
        }
    }

    std::vector<std::pair<int, int>> matched;
    std::vector<int> pending, free_sources, free_sinks;
    for (int source = 0; source < static_cast<int>(dag.size()); ++source)
    {
        if (incoming[source])
        {
            continue;
        }
        int sink = -1;
        pending.assign(1, source);
        while (!pending.empty())
        {
            const int vertex = pending.back();
            pending.pop_back();
            if (seen[vertex])
            {
                continue;
            }
            seen[vertex] = true;
            if (dag[vertex].empty())
            {
                sink = vertex;
                break;
            }
            pending.insert(pending.end(), dag[vertex].begin(), dag[vertex].end());
        }
        if (sink == -1)
        {
            free_sources.push_back(source);
        }
        else
        {
            matched.emplace_back(source, sink);
        }
    }
    for (int vertex = 0; vertex < static_cast<int>(dag.size()); ++vertex)
    {
        if (dag[vertex].empty() && !seen[vertex])
        {
            free_sinks.push_back(vertex);
        }
    }

    const std::size_t extras = std::max(free_sources.size(), free_sinks.size());
    std::vector<std::pair<int, int>> arcs;
    arcs.reserve(matched.size() + extras);
    auto append = [&](int from, int to)
    {
        arcs.emplace_back(groups[from].front(), groups[to].front());
    };
    for (std::size_t i = 0; i < matched.size(); ++i)
    {
        append(matched[i].second, matched[(i + 1) % matched.size()].first);
    }
    for (std::size_t i = 0; i < extras; ++i)
    {
        const int from = i < free_sinks.size() ? free_sinks[i] : matched.front().second;
        const int to = i < free_sources.size() ? free_sources[i] : matched.front().first;
        append(from, to);
    }
    return arcs;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
