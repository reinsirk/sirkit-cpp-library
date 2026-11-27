// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Rooted_Tree_Isomorphism.cpp"
#include <cassert>

int main()
{
    const std::vector<int> parent{-1, 0, 0};
    const auto result = sirkit::rooted_subtree_classes(parent);
    assert(result.count == 2 && result.class_id[1] == result.class_id[2]);
}
