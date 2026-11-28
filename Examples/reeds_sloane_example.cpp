// SPDX-License-Identifier: CC0-1.0
#include "../Math/Reeds_Sloane.cpp"

#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    const std::vector<std::uint32_t> sequence{0, 1, 1, 2, 3, 5, 0, 5};
    const auto coefficients = sirkit::reeds_sloane(sequence, 8);
    for (const auto coefficient : coefficients)
    {
        std::cout << coefficient << '\n'; // 1, 1
    }
}
