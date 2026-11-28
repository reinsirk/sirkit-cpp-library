// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Complement_Shortest_Paths.cpp"

#include <array>
#include <iostream>
#include <utility>

int main()
{
    const std::array removed{std::pair{0, 3}};
    const auto result = sirkit::complement_bfs(4, removed, 0, 998244353);
    std::cout << result.distance[3] << ' ' << result.paths[3] << '\n';
}
