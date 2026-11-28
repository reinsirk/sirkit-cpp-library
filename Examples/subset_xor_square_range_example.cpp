// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Subset_Xor_Square_Range.cpp"

#include <array>
#include <cstdint>
#include <iostream>

int main()
{
    const std::array<std::uint32_t, 2> values{1, 2};
    sirkit::SubsetXorSquareRange<> sums(values);
    std::cout << sums.sum_squares(0, 2) << '\n'; // 1^2 + 2^2 + 3^2 = 14
    sums.set(1, 1);
    std::cout << sums.sum_squares(0, 2) << '\n'; // 1^2 + 1^2 + 0^2 = 2
}
