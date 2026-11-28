// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Adjacent_Swap_Parity.cpp"

#include <array>
#include <iostream>

int main()
{
    const std::array from{1, 2, 3, 1}, to{1, 3, 2, 1};
    const auto result = sirkit::adjacent_swap_distances_by_parity(from, to);
    std::cout << result.even.value() << ' ' << result.odd.value() << '\n'; // 6 1
}
