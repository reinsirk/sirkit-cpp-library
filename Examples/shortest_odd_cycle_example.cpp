// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Shortest_Odd_Cycle.cpp"

#include <iostream>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}, {2, 0}, {2, 3}};
    const auto cycle = sirkit::shortest_odd_cycle(4, edges);
    std::cout << cycle.size() << '\n'; // 3
}
