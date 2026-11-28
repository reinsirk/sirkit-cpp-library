#ifndef SIRKIT_TREE_EDGE_COLOR_PATH_COUNTS_DEFINED
#define SIRKIT_TREE_EDGE_COLOR_PATH_COUNTS_DEFINED

struct TreeColorEdge
{
    int from;
    int to;
    std::int64_t color;
};

struct TreeEdgeColorPathCounts
{
    std::uint64_t one_color{};
    std::uint64_t two_colors{};
};

[[nodiscard]] inline TreeEdgeColorPathCounts tree_edge_color_path_counts(
    int n, std::span<const TreeColorEdge> edges)
{
    if (n < 0 || edges.size() != static_cast<std::size_t>(n == 0 ? 0 : n - 1))
    {
        throw std::length_error("tree_edge_color_path_counts: inconsistent tree size");
    }

    std::vector<std::int64_t> colors;
    colors.reserve(edges.size());
    for (const auto &edge : edges)
    {
        colors.push_back(edge.color);
    }
    std::ranges::sort(colors);
    colors.erase(std::unique(colors.begin(), colors.end()), colors.end());

    Graph graph(n);
    std::vector<std::vector<std::pair<int, int>>> colored(n);
    for (const auto &[u, v, label] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(u) >= graph.size() || static_cast<std::size_t>(v) >= graph.size())
        {
            throw std::out_of_range("tree_edge_color_path_counts: invalid vertex");
        }
#endif
        const int color = static_cast<int>(std::lower_bound(colors.begin(), colors.end(), label) - colors.begin());
        graph[u].push_back(v);
        graph[v].push_back(u);
        colored[u].emplace_back(v, color);
        colored[v].emplace_back(u, color);
    }

    const CentroidDecomposition layout(graph, false);
    std::vector<bool> removed(n);
    std::vector<int> one(colors.size()), two_containing(colors.size()), touched;
    std::map<std::pair<int, int>, int> two;
    struct Visit
    {
        int vertex;
        int parent;
        int first;
        int second;
    };
    std::vector<Visit> pending;
    std::vector<std::pair<int, int>> branch;
    TreeEdgeColorPathCounts result;
    for (const int centroid : layout.order())
    {
        removed[centroid] = true;
        int one_total = 0;
        for (const auto &[start, color] : colored[centroid])
        {
            if (removed[start])
            {
                continue;
            }
            branch.clear();
            pending.push_back({start, centroid, color, -1});
            while (!pending.empty())
            {
                const auto [v, previous, first, second] = pending.back();
                pending.pop_back();
                branch.emplace_back(first, second);
                for (const auto &[u, next_color] : colored[v])
                {
                    if (u == previous || removed[u])
                    {
                        continue;
                    }
                    if (next_color == first || next_color == second)
                    {
                        pending.push_back({u, v, first, second});
                    }
                    else if (second == -1)
                    {
                        const auto [low, high] = std::minmax(first, next_color);
                        pending.push_back({u, v, low, high});
                    }
                }
            }

            for (const auto &[first, second] : branch)
            {
                if (second == -1)
                {
                    result.one_color += 1 + one[first];
                    result.two_colors += one_total - one[first] + two_containing[first];
                }
                else
                {
                    result.two_colors += 1 + one[first] + one[second] + two[{first, second}];
                }
            }
            for (const auto &[first, second] : branch)
            {
                touched.push_back(first);
                if (second == -1)
                {
                    ++one_total;
                    ++one[first];
                }
                else
                {
                    touched.push_back(second);
                    ++two_containing[first];
                    ++two_containing[second];
                    ++two[{first, second}];
                }
            }
        }
        for (const int color : touched)
        {
            one[color] = two_containing[color] = 0;
        }
        touched.clear();
        two.clear();
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
