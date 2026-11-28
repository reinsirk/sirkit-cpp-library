// SPDX-License-Identifier: CC0-1.0
#include "../Math/Set_Series_Composition.cpp"
#include <array>
#include <iostream>

int main()
{
    const std::array<std::uint32_t, 3> polynomial{1, 2, 3};
    const std::array<std::uint32_t, 4> series{2, 3, 5, 7};
    const auto answer = sirkit::set_series_compose(polynomial, series);
    std::cout << answer[3] << '\n'; // 188
}
