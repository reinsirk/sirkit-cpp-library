// SPDX-License-Identifier: CC0-1.0
#include "../Math/Polynomial_Moments.cpp"

#include <iostream>
#include <vector>

int main()
{
    const auto moments = sirkit::weighted_power_sums(std::vector{2, 3}, std::vector{4, 5}, 3);
    std::cout << moments.get(0) << ' ' << moments.get(1) << ' ' << moments.get(2) << '\n'; // 9 23 61
    const sirkit::FormalPowerSeries polynomial{1, 2};
    const auto composition = sirkit::exponential_composition(polynomial, 1, 3, -1);
    std::cout << composition.get(0) << ' ' << composition.get(1) << ' ' << composition.get(2) << '\n'; // 1 2 1
}
