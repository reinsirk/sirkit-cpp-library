// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Shortest_Weighted_Cycle.cpp"

#include <array>
#include <cassert>

int main()
{
    using Edge = sirkit::WeightedCycleEdge;
    const std::array edges{Edge{0, 1, 3}, Edge{1, 2, 1}, Edge{2, 0, 2}, Edge{0, 1, 7}};
    const auto result = sirkit::shortest_weighted_cycle(3, edges);
    assert(result && result->weight == 6);
    assert(result->cycle.vertices.size() == 3);
}
