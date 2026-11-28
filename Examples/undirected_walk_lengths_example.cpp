// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Undirected_Walk_Lengths.cpp"

#include <iostream>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}, {0, 3}};
    const sirkit::UndirectedWalkLengths lengths(4, edges, 0);
    std::cout << lengths.can_walk_to(2, 3) << '\n'; // 0
    for (const int v : lengths.vertices_on_walks(2, 4))
    {
        std::cout << v << '\n'; // 0, 1, 2, 3
    }
}
