// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Target_Sum_Subrectangles.cpp"

#include <cassert>

int main()
{
    const int grid[2][2]{{1, -1}, {-1, 1}};
    const auto count = sirkit::count_subrectangles_with_sum(2, 2, 0, [&](int row, int col)
    {
        return grid[row][col];
    });
    assert(count == 5);
}
