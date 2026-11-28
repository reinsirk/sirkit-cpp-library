// SPDX-License-Identifier: CC0-1.0
#include "../Math/Tridiagonal_Matrix_Power.cpp"

#include <array>
#include <cstdint>
#include <iostream>

int main()
{
    const std::array<std::int64_t, 1> lower{1}, upper{1};
    const std::array<std::int64_t, 2> diagonal{1, 1};
    const auto column = sirkit::tridiagonal_power_first_column(2, lower, diagonal, upper);
    std::cout << column[0] << ' ' << column[1] << '\n'; // 2 2
}
