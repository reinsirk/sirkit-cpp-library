// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Forest_Subset_Components.cpp"

#include <array>
#include <iostream>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 3}};
    const auto counts = sirkit::forest_subset_components<int>(4, edges);
    for (const auto count : counts)
    {
        std::cout << count << ' ';
    }
    std::cout << '\n';
}
