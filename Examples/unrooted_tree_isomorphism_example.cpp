// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Unrooted_Tree_Isomorphism.cpp"

#include <cassert>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<int, int>> first{{0, 1}, {1, 2}, {2, 3}};
    const std::vector<std::pair<int, int>> second{{2, 3}, {3, 1}, {1, 0}};
    assert(sirkit::unrooted_trees_equal(4, first, second));

    const std::vector<std::pair<int, int>> shorter{{0, 1}, {0, 2}};
    assert((sirkit::tree_leaf_deletion_matches(4, first, shorter) == std::vector<int>{0, 3}));
}
