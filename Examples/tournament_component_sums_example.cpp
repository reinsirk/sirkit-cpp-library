// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tournament_Component_Sums.cpp"

#include <iostream>

int main()
{
    const auto sums = sirkit::tournament_component_sums<int>(3);
    std::cout << sums[1] << '\n'; // 7
}
