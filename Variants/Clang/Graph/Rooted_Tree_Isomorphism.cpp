// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Rooted_Tree_Isomorphism.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/rooted_tree_isomorphism.md
//
// API一覧
// rooted_subtree_classes(parent): 各頂点を根とする部分木を同型ごとに分類する。
// parent[v]は親の番号[0,n)または根を表す-1。森も指定可。兄弟順は無視する。
// RootedSubtreeClasses{count,class_id}: 分類数と各頂点の分類番号[0,count)。O(n log(n+1))時間・O(n)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
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
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
