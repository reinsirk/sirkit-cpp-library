// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Marked_Diameter.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{1, 3}};
    sirkit::TreeMarkedDiameter tree(4, edges);
    tree.set_marked(0);
    tree.set_marked(2);
    assert(tree.diameter()->distance == 2);
    assert(tree.subtree_diameter(1)->distance == 0);
    tree.toggle(2);
    assert(tree.farthest(3)->vertex == 0);
}
