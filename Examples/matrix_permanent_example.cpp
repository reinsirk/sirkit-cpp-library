// SPDX-License-Identifier: CC0-1.0
#include "../Math/Matrix_Permanent.cpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>

int main()
{
    const std::array grid{std::array{2, 3}, std::array{4, 5}};
    const auto answer = sirkit::matrix_permanent<std::int64_t>(grid.size(), [&](std::size_t row, std::size_t column)
                                                               {
                                                                   return grid[row][column];
                                                               });
    std::cout << answer << '\n'; // 2*5 + 3*4 = 22
}
