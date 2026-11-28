// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Forest_Coloring.cpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 3}};
    const sirkit::ForestColoring forest(4, edges);
    std::cout << forest.edge_coloring().color_count << '\n';          // 2
    std::cout << forest.distance_two_coloring().color_count << '\n';  // 3
    std::cout << forest.count_distance_two<std::uint64_t>(3) << '\n'; // 6
}
