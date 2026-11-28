// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Distance_Layers.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 3}};
    const sirkit::TreeDistanceLayers tree(4, edges);
    assert(tree.vertex_at_distance(1, 2) == 3);
    assert(tree.vertex_at_distance(1, 3) == -1);
    assert(tree.farthest(1) == 3 && tree.eccentricity(1) == 2);
}
