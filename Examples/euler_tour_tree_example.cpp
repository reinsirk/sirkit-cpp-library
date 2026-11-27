// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Euler_Tour_Tree.cpp"

#include <cassert>

int main()
{
    sirkit::EulerTourTree<> forest(3);
    assert(forest.link(0, 1));
    assert(forest.link(1, 2));
    assert(forest.subtree_add(1, 0, 4));
    assert(forest.subtree_sum(1, 0) == 8);
    assert(forest.cut(1, 2));
    assert(!forest.connected(0, 2));
    assert(forest.get(2) == 4);
}
