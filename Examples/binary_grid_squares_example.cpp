// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Binary_Grid_Squares.cpp"

#include <array>
#include <cassert>
#include <string_view>

int main()
{
    const std::array<std::string_view, 3> grid{"...", "...", "#.."};
    const auto result = sirkit::binary_grid_squares(3, 3, [&](int row, int col)
    {
        return grid[row][col] == '.';
    });
    assert(result.count == 11 && result.by_side[2] == 3);
    assert(result.largest && result.largest->side == 2 && result.largest->top == 0 && result.largest->left == 0);
}
