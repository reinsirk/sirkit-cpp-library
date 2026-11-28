// SPDX-License-Identifier: CC0-1.0
#include "../Math/Polynomial_Differential_Operator.cpp"

#include <iostream>

int main()
{
    const auto result = sirkit::polynomial_differential_operator_power({0, 1}, {-1, 0, 1}, {}, 3, 5);
    for (const auto coefficient : result.coefficients())
    {
        std::cout << coefficient << '\n'; // 2, 0, 998244345, 0, 6
    }
}
