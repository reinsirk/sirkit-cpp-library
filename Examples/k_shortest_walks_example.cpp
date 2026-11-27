// SPDX-License-Identifier: CC0-1.0
#include "../Graph/K_Shortest_Walks.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    using sirkit::KShortestWalkEdge;
    const std::array edges{
        KShortestWalkEdge{0, 1, 1},
        KShortestWalkEdge{0, 1, 1},
        KShortestWalkEdge{1, 0, 4},
    };
    const auto lengths = sirkit::k_shortest_walk_lengths(2, 0, 1, edges, 4);
    assert((lengths == std::vector<std::int64_t>{1, 1, 6, 6}));
}
