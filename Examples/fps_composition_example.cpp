// SPDX-License-Identifier: CC0-1.0
#include "../Math/FPS_Composition.cpp"
#include <iostream>

int main()
{
    const sirkit::FormalPowerSeries f{0, 1, -1}; // x-x^2
    const auto g = sirkit::fps_compositional_inverse(f, 5);
    std::cout << g[4] << '\n'; // 5
    const auto identity = sirkit::fps_compose(f, g, 5);
    std::cout << identity[1] << '\n'; // 1
    const auto projected = sirkit::fps_power_projection({2, 1}, {1, 2, 1}, 2, 4);
    std::cout << projected[3] << '\n'; // 38
}
