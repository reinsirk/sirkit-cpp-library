// SPDX-License-Identifier: CC0-1.0
#include "../Math/Modular_Shift_Sums.cpp"

#include <array>
#include <iostream>

int main()
{
    const std::array<sirkit::ModularShiftTerm, 2> terms{{{1, 3}, {2, 5}}};
    for (const auto sum : sirkit::modular_shift_sums(3, terms))
    {
        std::cout << sum << '\n'; // 3, 5, 4, 1
    }
}
