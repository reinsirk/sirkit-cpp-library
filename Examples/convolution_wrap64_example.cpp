// SPDX-License-Identifier: CC0-1.0
#include "../Math/Convolution_Wrap64.cpp"
#include <array>
#include <iostream>
#include <limits>

int main()
{
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const std::array<std::uint64_t, 2> a{kMax, 2}, b{3, 4};
    std::cout << sirkit::convolution_mod_2_64(a, b)[1] << '\n'; // (-1)*4+2*3 = 2
}
