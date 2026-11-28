// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Dynamic_Tree_Distances.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array edges{sirkit::DynamicTreeDistanceEdge<>{0, 1, 4},
                           sirkit::DynamicTreeDistanceEdge<>{1, 2, 6}};
    sirkit::DynamicTreeDistances<> tree(3, edges);
    assert(tree.distance(0, 2) == 10);
    tree.set_weight(1, 8);
    assert(tree.distance(0, 2) == 12 && tree.root_distance(2) == 12);
}
