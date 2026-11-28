#ifndef SIRKIT_UNROOTED_TREE_ISOMORPHISM_DEFINED
#define SIRKIT_UNROOTED_TREE_ISOMORPHISM_DEFINED

namespace detail
{
using TreeShapeEdge = std::pair<int, int>;

inline void check_tree_shape_size(int n)
{
    if (n < 0 || n > std::numeric_limits<int>::max() / 4)
    {
        throw std::length_error("unrooted tree isomorphism: vertex count");
    }
}

class TreeShapeRoots
{
public:
    TreeShapeRoots(int n, std::span<const TreeShapeEdge> edges)
        : graph_(n)
    {
#ifndef SKIP_RUNTIME_CHECKS
        const int count = std::max(0, n - 1);
        if (edges.size() != static_cast<std::size_t>(count))
        {
            throw std::invalid_argument("unrooted tree isomorphism: number of edges");
        }
#endif
        for (const auto &[u, v] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (static_cast<unsigned>(u) >= static_cast<unsigned>(n) || static_cast<unsigned>(v) >= static_cast<unsigned>(n))
            {
                throw std::out_of_range("unrooted tree isomorphism: edge endpoint");
            }
#endif
            graph_[u].push_back(v);
            graph_[v].push_back(u);
        }
        if (n == 0)
        {
            return;
        }

        std::vector<int> parent(n, -2);
        const auto order = orient(0, 0, parent);
        std::vector<int> subtree(n, 1), largest_child(n);
        for (auto i = order.rbegin(); i != order.rend(); ++i)
        {
            const int v = *i;
            const int p = parent[v];
            if (p != -1)
            {
                subtree[p] += subtree[v];
                largest_child[p] = std::max(largest_child[p], subtree[v]);
            }
        }
        for (int v = 0; v < n; ++v)
        {
            if (std::max(largest_child[v], n - subtree[v]) <= n / 2)
            {
                centers_.push_back(v);
            }
        }
    }

    [[nodiscard]] std::vector<int> append_parents(std::vector<int> &combined) const
    {
        std::vector<int> roots;
        for (int center : centers_)
        {
            const int base = static_cast<int>(combined.size());
            combined.resize(combined.size() + graph_.size(), -2);
            (void)orient(center, base, std::span<int>(combined).subspan(base));
            roots.push_back(base + center);
        }
        return roots;
    }

private:
    [[nodiscard]] std::vector<int> orient(int root, int offset, std::span<int> parent) const
    {
        std::vector<int> order;
        order.reserve(graph_.size());
        order.push_back(root);
        parent[root] = -1;
        for (std::size_t i = 0; i < order.size(); ++i)
        {
            const int v = order[i];
            for (int w : graph_[v])
            {
                if (w + offset == parent[v])
                {
                    continue;
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (parent[w] != -2)
                {
                    throw std::invalid_argument("unrooted tree isomorphism: cycle");
                }
#endif
                parent[w] = v + offset;
                order.push_back(w);
            }
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (order.size() != graph_.size())
        {
            throw std::invalid_argument("unrooted tree isomorphism: disconnected");
        }
#endif
        return order;
    }

    std::vector<std::vector<int>> graph_;
    std::vector<int> centers_;
};

struct TreeChildDifference
{
    int extra_class = -1;
    int missing = -1;
};

inline std::optional<TreeChildDifference> tree_child_difference(
    std::span<const int> left, std::span<const int> right, std::span<const int> classes)
{
    if (left.size() < right.size() || left.size() - right.size() > 1)
    {
        return std::nullopt;
    }
    TreeChildDifference difference;
    std::size_t i = 0, j = 0;
    while (i < left.size() || j < right.size())
    {
        if (i < left.size() && j < right.size() && classes[left[i]] == classes[right[j]])
        {
            ++i;
            ++j;
        }
        else if (j == right.size() || (i < left.size() && classes[left[i]] < classes[right[j]]))
        {
            if (difference.extra_class != -1)
            {
                return std::nullopt;
            }
            difference.extra_class = classes[left[i++]];
        }
        else
        {
            if (difference.missing != -1)
            {
                return std::nullopt;
            }
            difference.missing = right[j++];
        }
    }
    return difference;
}
}

inline bool unrooted_trees_equal(int n, std::span<const std::pair<int, int>> first, std::span<const std::pair<int, int>> second)
{
    detail::check_tree_shape_size(n);
    std::vector<int> parents;
    const auto a = detail::TreeShapeRoots(n, first).append_parents(parents);
    const auto b = detail::TreeShapeRoots(n, second).append_parents(parents);
    if (n == 0)
    {
        return true;
    }
    const auto classes = rooted_subtree_classes(parents).class_id;
    for (int x : a)
    {
        for (int y : b)
        {
            if (classes[x] == classes[y])
            {
                return true;
            }
        }
    }
    return false;
}

inline std::vector<int> tree_leaf_deletion_matches(
    int n, std::span<const std::pair<int, int>> tree, std::span<const std::pair<int, int>> smaller)
{
    detail::check_tree_shape_size(n);
    if (n < 2)
    {
        throw std::invalid_argument("tree_leaf_deletion_matches: at least two vertices required");
    }
    std::vector<int> parents;
    const auto a = detail::TreeShapeRoots(n, tree).append_parents(parents);
    const auto b = detail::TreeShapeRoots(n - 1, smaller).append_parents(parents);
    const auto classes = rooted_subtree_classes(parents).class_id;
    std::vector<std::vector<int>> children(parents.size());
    for (int v = 0; v < static_cast<int>(parents.size()); ++v)
    {
        if (parents[v] != -1)
        {
            children[parents[v]].push_back(v);
        }
    }
    for (auto &group : children)
    {
        std::ranges::sort(group, {}, [&](int v)
                          {
                              return classes[v];
                          });
    }

    std::vector<bool> matching(n);
    std::vector<std::pair<int, int>> pending;
    for (int x : a)
    {
        for (int y : b)
        {
            pending.emplace_back(x, y);
            while (!pending.empty())
            {
                const auto [u, v] = pending.back();
                pending.pop_back();
                const auto difference = detail::tree_child_difference(children[u], children[v], classes);
                if (!difference)
                {
                    continue;
                }
                for (int child : children[u])
                {
                    if (classes[child] == difference->extra_class)
                    {
                        if (difference->missing == -1)
                        {
                            matching[child % n] = true;
                        }
                        else
                        {
                            pending.emplace_back(child, difference->missing);
                        }
                    }
                }
            }
        }
    }
    std::vector<int> result;
    for (int v = 0; v < n; ++v)
    {
        if (matching[v])
        {
            result.push_back(v);
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
