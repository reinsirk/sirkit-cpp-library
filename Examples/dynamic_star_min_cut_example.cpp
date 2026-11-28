// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Dynamic_Star_Min_Cut.cpp"

#include <array>
#include <iostream>

int main()
{
    const std::array edges{sirkit::DynamicStarCutEdge{0, 1, 5}};
    sirkit::DynamicStarMinCut cut(2, edges);
    std::cout << cut.set_star_weight(0, 2) << '\n';
    std::cout << cut.set_star_weight(1, 10) << '\n';
}
