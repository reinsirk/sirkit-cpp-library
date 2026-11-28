// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Shortest_Path_Edge_Usage.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array edges{sirkit::ShortestPathEdge{0, 1, 2},
                           sirkit::ShortestPathEdge{1, 2, 3},
                           sirkit::ShortestPathEdge{2, 3, 1},
                           sirkit::ShortestPathEdge{0, 2, 5},
                           sirkit::ShortestPathEdge{1, 3, 7}};
    const auto routes = sirkit::shortest_path_edge_usage(4, edges, 0, 3);
    assert(routes.distance == 6);
    assert(routes.edge_use[0] == sirkit::ShortestPathEdgeUse::some);
    assert(routes.edge_use[2] == sirkit::ShortestPathEdgeUse::all);
    assert(routes.edge_use[4] == sirkit::ShortestPathEdgeUse::none);
}
