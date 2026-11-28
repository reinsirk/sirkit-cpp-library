// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Strong_Connectivity_Augmentation.cpp"

#include <array>
#include <iostream>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{2, 1}};
    const auto added = sirkit::strong_connectivity_augmentation(3, edges);
    for (const auto &[from, to] : added)
    {
        std::cout << from << ' ' << to << '\n';
    }
}
