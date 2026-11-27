// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Minimum_Diameter_Spanning_Tree.cpp"

#include <array>
#include <cassert>

int main()
{
    using Edge = sirkit::MinimumDiameterTreeEdge;
    const std::array edges{Edge{0, 1, 1}, Edge{1, 2, 2}, Edge{0, 2, 9}};
    const auto tree = sirkit::minimum_diameter_spanning_tree(3, edges);
    assert(tree && tree->diameter == 3 && tree->edge_ids.size() == 2);
}
