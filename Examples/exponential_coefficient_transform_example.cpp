// SPDX-License-Identifier: CC0-1.0
#include "../Math/Exponential_Coefficient_Transform.cpp"

#include <iostream>

int main()
{
    const sirkit::FormalPowerSeries series{1, 2};
    const auto transformed = sirkit::exponential_coefficient_transform(series, 1, 3);
    std::cout << transformed[0] << ' ' << transformed[1] << ' ' << transformed[2] << '\n'; // 1 3 6
    const auto restored = sirkit::inverse_exponential_coefficient_transform(transformed, 1, 3);
    std::cout << restored[0] << ' ' << restored[1] << ' ' << restored[2] << '\n'; // 1 2 0
}
