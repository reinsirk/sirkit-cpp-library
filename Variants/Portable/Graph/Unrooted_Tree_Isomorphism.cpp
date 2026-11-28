// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Unrooted_Tree_Isomorphism.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/unrooted_tree_isomorphism.md
//
// API一覧
// unrooted_trees_equal(n,first,second): 2つのn頂点の無向木の同型判定。n=0の両空木は同型。
// tree_leaf_deletion_matches(n,tree,smaller): n頂点の木から葉1つを消して、n-1頂点の木と同型にできる葉を昇順に返す。n>=2。
//   頂点は0始まり。無向辺をpair<int,int>で1回渡す。両関数とも n<=INT_MAX/4。
//   根の候補を重心に絞り、部分木の同型分類を共有する。衝突のない決定的な判定。O(n log(n+1))時間、O(n)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ROOTED_TREE_ISOMORPHISM_DEFINED
#define SIRKIT_ROOTED_TREE_ISOMORPHISM_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline int rooted_tree_vertex_count(std::size_t size)
{
    if (size > std::numeric_limits<int>::max())
    {
        throw std::length_error("rooted_subtree_classes: too many vertices");
    }
    return static_cast<int>(size);
}
} // namespace detail

struct RootedSubtreeClasses
{
    int count;
    std::vector<int> class_id;
};

inline RootedSubtreeClasses rooted_subtree_classes(std::span<const int> parent)
{
    const auto n = detail::rooted_tree_vertex_count(parent.size());
    std::vector<int> remaining(n);
    for (int p : parent)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (p < -1 || p >= n)
        {
            throw std::out_of_range("rooted_subtree_classes: invalid parent");
        }
#endif
        if (p != -1)
        {
            ++remaining[p];
        }
    }
    std::vector<int> order;
    order.reserve(n);
    for (int v = 0; v < n; ++v)
    {
        if (remaining[v] == 0)
        {
            order.push_back(v);
        }
    }

    std::vector<std::vector<int>> signature(n);
    std::map<std::vector<int>, int> classes;
    RootedSubtreeClasses result{0, std::vector<int>(n)};
    for (std::size_t cursor = 0; cursor < order.size(); ++cursor)
    {
        const int v = order[cursor];
        auto &key = signature[v];
        std::ranges::sort(key);
        const auto [pos, inserted] = classes.try_emplace(std::move(key), result.count);
        result.count += inserted;
        result.class_id[v] = pos->second;
        const int p = parent[v];
        if (p != -1)
        {
            signature[p].push_back(pos->second);
            if (--remaining[p] == 0)
            {
                order.push_back(p);
            }
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (order.size() != parent.size())
    {
        throw std::invalid_argument("rooted_subtree_classes: parent cycle");
    }
#endif
    return result;
}

#endif

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
