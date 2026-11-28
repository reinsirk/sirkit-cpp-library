// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Edge_Color_Path_Counts.cpp"

#include <iostream>
#include <vector>

int main()
{
    const std::vector<sirkit::TreeColorEdge> edges{{0, 1, 7}, {1, 2, 7}, {2, 3, 9}};
    const auto counts = sirkit::tree_edge_color_path_counts(4, edges);
    std::cout << counts.one_color << ' ' << counts.two_colors << '\n'; // 4 2
}
