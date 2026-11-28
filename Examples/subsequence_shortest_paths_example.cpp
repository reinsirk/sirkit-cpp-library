// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Subsequence_Shortest_Paths.cpp"

#include <array>
#include <iostream>

int main()
{
    using Edge = sirkit::SubsequencePathEdge<int>;
    const std::array edges{Edge{0, 1, 4}, Edge{1, 2, -2}, Edge{0, 2, 5}};
    const std::array schedule{1, 2, 0, 1};
    const sirkit::SubsequenceShortestPaths<int> paths(3, edges, schedule, 0);
    std::cout << paths.distance_to(2).value() << '\n'; // 2
    const auto result = paths.path_to(2).value();
    for (const auto &[at, id] : result)
    {
        std::cout << at << ' ' << id << '\n'; // 2 0, then 3 1
    }
}
