// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Nonadjacent_Selection.cpp"

#include <array>
#include <iostream>

int main()
{
    const std::array weights{4, 8, 6};
    const sirkit::NonadjacentSelection profile(weights);
    std::cout << profile.sums()[2] << '\n'; // 10
    const auto positions = profile.positions(2);
    for (const auto index : *positions)
    {
        std::cout << index << '\n'; // 0, 2
    }
}
