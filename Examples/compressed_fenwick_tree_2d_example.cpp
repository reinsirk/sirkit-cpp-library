// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Compressed_Fenwick_Tree_2D.cpp"

#include <cassert>
#include <vector>

int main()
{
    using Tree = sirkit::CompressedFenwickTree2D<>;
    const std::vector<Tree::Point> coordinates{{-10, 5}, {2, 1}, {2, 9}};
    Tree tree(coordinates);
    assert(tree.add({-10, 5}, 4));
    assert(tree.add({2, 1}, 7));
    assert(tree.add({2, 9}, -3));
    assert(tree.sum(-10, 1, 3, 9) == 11);
    assert(!tree.add({0, 0}, 2));
}
