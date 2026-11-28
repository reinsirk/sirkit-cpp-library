// SPDX-License-Identifier: CC0-1.0
#include "../Math/Polynomial_Roots.cpp"
#include <iostream>

int main()
{
    const sirkit::FormalPowerSeries polynomial{0, -1, 1}; // x^2 - x
    for (const auto root : sirkit::polynomial_roots(polynomial))
    {
        std::cout << root << '\n'; // 0, 1
    }
}
