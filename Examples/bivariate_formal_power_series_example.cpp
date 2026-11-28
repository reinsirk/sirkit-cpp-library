// SPDX-License-Identifier: CC0-1.0
#include "../Math/Bivariate_Formal_Power_Series.cpp"
#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    const std::vector<std::uint32_t> coefficients{1, 998244352, 998244352, 0};
    const auto inverse = sirkit::bivariate_fps_inverse(coefficients, 2, 2);
    std::cout << inverse[3] << '\n'; // 2
}
