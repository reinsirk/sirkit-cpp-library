// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Superincreasing_Path_Queries.cpp"

#include <array>
#include <iostream>

int main()
{
    using Edge = sirkit::SuperincreasingPathEdge<int>;
    const std::array edges{Edge{0, 1, 2}, Edge{1, 2, 4}, Edge{0, 2, 8}};
    const sirkit::SuperincreasingPathQueries<int> path(3, edges);
    std::cout << path.path_sum(0, 2).value() << '\n';
    std::cout << path.path_sum_without_edge(0, 2, 0).value() << '\n';
}
