// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Neighborhood_Assignment.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    using Edge = std::pair<int, int>;
    const std::array initial{1, 2, 3};
    const std::array edges{Edge{0, 1}, Edge{1, 2}};
    sirkit::NeighborhoodAssignment<int> colors(initial, edges);
    colors.assign_neighbors(1, 8);
    assert(colors.get(0) == 8 && colors.get(1) == 2 && colors.get(2) == 8);
    colors.assign(2, 9);
    assert(colors.get(2) == 9);
}
