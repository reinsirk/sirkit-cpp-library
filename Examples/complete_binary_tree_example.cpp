// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Complete_Binary_Tree.cpp"

#include <cassert>
#include <cstdint>
#include <utility>

int main()
{
    const sirkit::CompleteBinaryTree tree(10);
    assert(tree.lca(5, 8) == 2 && tree.distance(5, 8) == 3);
    assert(tree.kth_on_path(5, 8, 2) == 4);
    assert(tree.count_at_distance(2, 2) == 4);
    const std::pair<std::uint64_t, std::uint64_t> expected{8, 10};
    assert(tree.descendant_range(2, 2) == expected);
}
