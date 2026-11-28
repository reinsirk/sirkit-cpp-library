// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Walk_Orientation.cpp"

#include <array>
#include <iostream>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
    const std::array walks{std::pair{0, 2}, std::pair{0, 2}};
    const auto solution = sirkit::orient_tree_walks(3, edges, walks);
    std::cout << solution.covered_directions << '\n'; // 4
    for (std::size_t i = 0; i < walks.size(); ++i)
    {
        auto [u, v] = walks[i];
        if (solution.reversed[i])
        {
            std::swap(u, v);
        }
        std::cout << u << ' ' << v << '\n';
    }
}
