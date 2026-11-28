// SPDX-License-Identifier: CC0-1.0
#include "../Math/Bounded_Nondecreasing_Sequences.cpp"
#include <array>
#include <iostream>

int main()
{
    const std::array<std::uint32_t, 2> lower{0, 1}, upper{2, 3};
    // (0,1), (0,2), (1,1), (1,2)
    std::cout << sirkit::count_bounded_nondecreasing_sequences(lower, upper) << '\n'; // 4
}
