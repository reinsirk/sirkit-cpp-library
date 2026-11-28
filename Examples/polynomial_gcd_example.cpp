// SPDX-License-Identifier: CC0-1.0
#include "../Math/Polynomial_GCD.cpp"
#include <iostream>

int main()
{
    const sirkit::FormalPowerSeries x{0, 1};
    const sirkit::FormalPowerSeries modulus{-1, 0, 1}; // x^2 - 1
    if (const auto inverse = sirkit::polynomial_inverse_mod(x, modulus))
    {
        std::cout << inverse->get(1) << '\n'; // xの逆元もx
    }
}
