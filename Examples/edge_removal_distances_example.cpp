// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Edge_Removal_Distances.cpp"

#include <array>
#include <cassert>
#include <utility>
#include <vector>

int main()
{
    using Edge = std::pair<int, int>;
    const std::array edges{Edge{0, 1}, Edge{0, 2}, Edge{1, 2}};
    const auto distances = sirkit::edge_removal_distances(3, edges, 0, 2);
    assert(distances == std::vector<int>({1, 2, 1}));
}
