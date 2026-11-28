// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Core_Decomposition.cpp"

#include <array>
#include <iostream>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}, std::pair{1, 3}};
    const auto cores = sirkit::core_decomposition(4, edges);
    for (const auto number : cores.number)
    {
        std::cout << number << ' '; // 2 2 2 1
    }
    std::cout << '\n';
    const std::array weights{4LL, 5LL, 2LL, 3LL};
    std::cout << sirkit::weighted_core_decomposition<long long>(4, edges, weights).degeneracy << '\n'; // 6
}
