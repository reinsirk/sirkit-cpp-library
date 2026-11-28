// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Binary_Tree_Traversal.cpp"

#include <array>
#include <iostream>

int main()
{
    constexpr std::array preorder{1, 0, 2};
    constexpr std::array inorder{0, 1, 2};
    const auto tree = sirkit::binary_tree_from_preorder_inorder(preorder, inorder);
    const auto [left, right] = tree->children[tree->root];
    std::cout << tree->root << ' ' << left << ' ' << right << '\n'; // 1 0 2
}
