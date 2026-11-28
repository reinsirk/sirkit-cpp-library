// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Dice_Orientation.cpp"

#include <array>
#include <iostream>

int main()
{
    constexpr std::array labels{1, 6, 5, 2, 3, 4};
    constexpr sirkit::DiceOrientation start;
    const auto moved = start.rolled(sirkit::DiceRoll::North).apply(labels);
    std::cout << moved[0] << '\n'; // 2
}
