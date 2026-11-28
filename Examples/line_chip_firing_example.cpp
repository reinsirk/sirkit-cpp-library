// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Line_Chip_Firing.cpp"

#include <array>
#include <cstdint>
#include <iostream>

int main()
{
    constexpr std::array<std::int64_t, 3> chips{0, 0, 0};
    const auto result = sirkit::line_chip_firing(chips);
    std::cout << result.positions.front() << ' ' << result.operations << '\n'; // -1 1
}
