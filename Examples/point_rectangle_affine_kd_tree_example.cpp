// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Point_Rectangle_Affine_KD_Tree.cpp"

#include <cassert>
#include <vector>

int main()
{
    using Tree = sirkit::PointRectangleAffineKDTree<>;
    const std::vector<Tree::Point> points{{1, 2}, {1, 2}};
    Tree tree(points);
    tree.set(0, 4);
    tree.range_affine({0, 0, 3, 3}, 2, 3);
    tree.set(1, 5);
    assert(tree.range_sum({0, 0, 3, 3}) == 16);
    assert(tree.active_count() == 2);
}
