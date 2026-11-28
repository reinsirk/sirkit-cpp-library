// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Binary_Tree_Traversal.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/binary_tree_traversal.md
//
// API一覧
// binary_tree_from_preorder_inorder(preorder,inorder): 行きがけ順と通りがけ順から二分木を復元。
// binary_tree_from_postorder_inorder(postorder,inorder): 帰りがけ順と通りがけ順から復元。
//   列は0..n-1の順列。対応する木がなければnullopt。どちらもO(n)時間・領域、再帰なし。
// IndexedBinaryTree{root,children}: children[v] = {左の子,右の子}。子がなければ-1。空木のrootは-1。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BINARY_TREE_TRAVERSAL_DEFINED
#define SIRKIT_BINARY_TREE_TRAVERSAL_DEFINED

struct IndexedBinaryTree
{
    int root = -1;
    std::vector<std::array<int, 2>> children;
};

namespace detail
{
inline int checked_binary_tree_size(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("binary_tree_from_traversals: too many vertices");
    }
    return static_cast<int>(size);
}

inline std::optional<IndexedBinaryTree> binary_tree_from_traversals(
    std::span<const int> order, std::span<const int> inorder, bool postorder)
{
    if (order.size() != inorder.size())
    {
        return std::nullopt;
    }
    const int n = checked_binary_tree_size(order.size());
    IndexedBinaryTree tree;
    if (n == 0)
    {
        return tree;
    }
    std::vector<int> position(n, -1);
    for (int i = 0; i < n; ++i)
    {
        const int v = inorder[i];
        if (v < 0 || v >= n || position[v] != -1)
        {
            return std::nullopt;
        }
        position[v] = i;
    }

    struct Frame
    {
        int first;
        int last;
        int start;
        int parent;
        int side;
    };
    tree.children.assign(n, {-1, -1});
    std::vector<Frame> pending{{0, n, 0, -1, 0}};
    while (!pending.empty())
    {
        const auto [first, last, start, parent, side] = pending.back();
        pending.pop_back();
        const int v = order[start + (postorder ? last - first - 1 : 0)];
        if (v < 0 || v >= n)
        {
            return std::nullopt;
        }
        const int middle = position[v];
        if (middle < first || middle >= last)
        {
            return std::nullopt;
        }

        if (parent == -1)
        {
            tree.root = v;
        }
        else
        {
            tree.children[parent][side] = v;
        }

        const int left_start = start + (postorder ? 0 : 1);
        if (middle + 1 < last)
        {
            const int right_start = left_start + (middle - first);
            pending.push_back({middle + 1, last, right_start, v, 1});
        }
        if (first < middle)
        {
            pending.push_back({first, middle, left_start, v, 0});
        }
    }
    return tree;
}
} // namespace detail

[[nodiscard]] inline std::optional<IndexedBinaryTree> binary_tree_from_preorder_inorder(
    std::span<const int> preorder, std::span<const int> inorder)
{
    return detail::binary_tree_from_traversals(preorder, inorder, false);
}

[[nodiscard]] inline std::optional<IndexedBinaryTree> binary_tree_from_postorder_inorder(
    std::span<const int> postorder, std::span<const int> inorder)
{
    return detail::binary_tree_from_traversals(postorder, inorder, true);
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
