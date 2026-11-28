// SPDX-License-Identifier: CC0-1.0
#include "../Math/Weighted_Taylor_Shifts.cpp"

#include <iostream>
#include <vector>

int main()
{
    const sirkit::FormalPowerSeries polynomial{0, 0, 1};
    const std::vector<int> offsets{-1, 1}, weights{1, 1};
    const auto sum = sirkit::weighted_taylor_shifts(polynomial, offsets, weights);
    std::cout << sum[0] << ' ' << sum[1] << ' ' << sum[2] << '\n'; // 2 0 2
}
