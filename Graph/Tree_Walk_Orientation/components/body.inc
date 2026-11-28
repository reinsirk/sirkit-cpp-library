#ifndef SIRKIT_TREE_WALK_ORIENTATION_DEFINED
#define SIRKIT_TREE_WALK_ORIENTATION_DEFINED

struct TreeWalkOrientationResult
{
    std::uint64_t covered_directions;
    std::vector<bool> reversed;
};

[[nodiscard]] inline TreeWalkOrientationResult orient_tree_walks(
    int n, std::span<const std::pair<int, int>> edges, std::span<const std::pair<int, int>> walks)
{
    if (n < 0 || n > (std::numeric_limits<int>::max() - 1) / 2)
    {
        throw std::length_error("orient_tree_walks: vertex count outside LCA range");
    }
    if (edges.size() != static_cast<std::size_t>(n == 0 ? 0 : n - 1))
    {
        throw std::invalid_argument("orient_tree_walks: wrong number of tree edges");
    }
    const auto check_vertex = [n](int v)
    {
        if (v < 0 || v >= n)
        {
            throw std::out_of_range("orient_tree_walks: vertex");
        }
    };
    for (const auto &[u, v] : walks)
    {
        check_vertex(u);
        check_vertex(v);
    }
    if (n == 0)
    {
        return {0, {}};
    }

    std::vector<std::vector<int>> tree(n);
    for (const auto &[u, v] : edges)
    {
        check_vertex(u);
        check_vertex(v);
        tree[u].push_back(v);
        tree[v].push_back(u);
    }
    constexpr int kUnvisited = -2;
    std::vector<int> parents(n, kUnvisited), order{0};
    parents[0] = -1;
    order.reserve(n);
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        const int v = order[i];
        for (const int w : tree[v])
        {
            if (parents[w] == kUnvisited)
            {
                parents[w] = v;
                order.push_back(w);
            }
        }
    }
    if (order.size() != static_cast<std::size_t>(n))
    {
        throw std::invalid_argument("orient_tree_walks: graph is disconnected");
    }

    const LinearLCA lca(parents);
    std::vector<std::int64_t> crossing(n);
    for (const auto &[u, v] : walks)
    {
        ++crossing[u];
        ++crossing[v];
        crossing[lca(u, v)] -= 2;
    }

    constexpr auto kNoWalk = std::numeric_limits<std::size_t>::max();
    struct Walk
    {
        int first, second;
        std::size_t combined;
        bool flip_with_combined;
        bool active;
    };
    std::vector<Walk> paths;
    paths.reserve(walks.size() * 2);
    std::vector<std::vector<std::size_t>> touches(n);
    const auto add = [&](int a, int b)
    {
        const auto id = paths.size();
        paths.push_back({a, b, kNoWalk, false, a != b});
        if (a != b)
        {
            touches[a].push_back(id);
            touches[b].push_back(id);
        }
        return id;
    };
    for (const auto &[u, v] : walks)
    {
        add(u, v);
    }

    std::uint64_t optimum = 0;
    for (std::size_t i = order.size() - 1; i != 0; --i)
    {
        const int v = order[i];
        const int parent = parents[v];
        optimum += static_cast<std::uint64_t>(std::min<std::int64_t>(2, crossing[v]));
        crossing[parent] += crossing[v];

        auto leftover = kNoWalk;
        for (const auto current : touches[v])
        {
            if (!paths[current].active)
            {
                continue;
            }
            if (leftover == kNoWalk)
            {
                leftover = current;
                continue;
            }
            const bool flip_left = paths[leftover].first == v;
            const bool flip_right = paths[current].second == v;
            const int a = flip_left ? paths[leftover].second : paths[leftover].first;
            const int b = flip_right ? paths[current].first : paths[current].second;
            const auto combined = add(a, b);
            paths[leftover].active = paths[current].active = false;
            paths[leftover].combined = paths[current].combined = combined;
            paths[leftover].flip_with_combined = flip_left;
            paths[current].flip_with_combined = flip_right;
            leftover = kNoWalk;
        }
        if (leftover != kNoWalk)
        {
            auto &path = paths[leftover];
            if (path.first == v)
            {
                path.first = parent;
            }
            else
            {
                path.second = parent;
            }
            path.active = path.first != path.second;
            if (path.active)
            {
                touches[parent].push_back(leftover);
            }
        }
    }

    std::vector<bool> reversed(paths.size());
    for (auto at = paths.size(); at != 0; --at)
    {
        const auto &record = paths[at - 1];
        if (record.combined != kNoWalk)
        {
            reversed[at - 1] = reversed[record.combined] != record.flip_with_combined;
        }
    }
    reversed.resize(walks.size());
    return {optimum, std::move(reversed)};
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
