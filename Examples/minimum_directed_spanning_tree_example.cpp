// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Minimum_Directed_Spanning_Tree.cpp"

#include <array>
#include <cassert>

int main()
{
    using sirkit::DirectedMstEdge;
    const std::array edges{
        DirectedMstEdge{0, 1, 10},
        DirectedMstEdge{0, 2, 8},
        DirectedMstEdge{1, 2, 1},
        DirectedMstEdge{2, 1, 2},
    };
    const auto result = sirkit::minimum_directed_spanning_tree(3, 0, edges);
    assert(result && result->cost == 10);
    assert(edges[result->incoming_edge[1]].from == 2);
}
