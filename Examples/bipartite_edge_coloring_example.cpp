// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Bipartite_Edge_Coloring.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 0}, std::pair{0, 1}, std::pair{1, 1}};
    const auto result = sirkit::bipartite_edge_coloring(2, 2, edges);
    assert(result.color_count == 2 && result.colors.size() == 3);
    assert(result.colors[0] != result.colors[1]);
    assert(result.colors[1] != result.colors[2]);
}
