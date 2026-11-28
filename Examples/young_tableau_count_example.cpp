// SPDX-License-Identifier: CC0-1.0
#include "../Math/Young_Tableau_Count.cpp"

#include <array>
#include <cstdint>
#include <iostream>

int main()
{
    constexpr std::array<std::uint32_t, 2> ordinary{3, 2};
    constexpr std::array<std::uint32_t, 2> domino{3, 3};
    std::cout << sirkit::standard_young_tableau_count(ordinary) << '\n'; // 5
    std::cout << sirkit::domino_tableau_count(domino) << '\n';           // 3
}
