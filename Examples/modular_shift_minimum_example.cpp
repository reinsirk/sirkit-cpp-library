// SPDX-License-Identifier: CC0-1.0
#include "../Math/Modular_Shift_Minimum.cpp"

#include <array>
#include <cstdint>
#include <iostream>

int main()
{
    const std::array<std::int64_t, 2> offsets{1, 3};
    std::cout << sirkit::sum_minimum_modular_shifts(3, 5, 3, offsets) << '\n'; // 4
}
