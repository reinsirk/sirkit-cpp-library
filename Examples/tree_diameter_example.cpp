// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Diameter.cpp"
#include <iostream>
#include <vector>

int main()
{
    const std::vector<sirkit::TreeDiameterEdge> edges{{0, 1, 4}, {1, 2, 3}, {1, 3, 8}};
    const auto result = sirkit::tree_diameter(4, edges);
    std::cout << result.length << '\n';
    for (const int vertex : result.path)
    {
        std::cout << vertex << ' ';
    }
    std::cout << '\n';
}
