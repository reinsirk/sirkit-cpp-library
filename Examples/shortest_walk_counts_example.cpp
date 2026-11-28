// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Shortest_Walk_Counts.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array edges{sirkit::ShortestPathEdge{0, 1, 1}, sirkit::ShortestPathEdge{0, 2, 1},
                           sirkit::ShortestPathEdge{1, 3, 1}, sirkit::ShortestPathEdge{2, 3, 1},
                           sirkit::ShortestPathEdge{3, 4, 0}, sirkit::ShortestPathEdge{4, 4, 0}};
    const auto walks = sirkit::count_shortest_walks(6, edges, 0);
    assert(walks.distance[3] == 2 && walks.count[3] == 2);
    assert(walks.infinite[4] && !walks.distance[5]);
}
