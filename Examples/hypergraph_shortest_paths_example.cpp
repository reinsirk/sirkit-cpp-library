// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Hypergraph_Shortest_Paths.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array first{0, 2};
    const std::array second{2, 4, 5};
    const std::array groups{sirkit::Hyperedge{first}, sirkit::Hyperedge{second}};
    const sirkit::HypergraphShortestPaths paths(6, groups);
    const auto distances = paths.unit_distances(0);
    assert(distances[4] == 2);
    assert(distances[1] == -1);
}
