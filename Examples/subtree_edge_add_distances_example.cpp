// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Subtree_Edge_Add_Distances.cpp"

#include <cassert>
#include <vector>

int main()
{
    using Tree = sirkit::SubtreeEdgeAddDistances<>;
    const std::vector<Tree::Edge> edges{{0, 1, 2}, {1, 2, 3}, {0, 3, 4}};
    Tree tree(4, edges);
    tree.add_subtree(1, 10);
    assert(tree.root_distance(1) == 2);
    assert(tree.root_distance(2) == 15);
    assert(tree.distance(2, 3) == 19);
}
