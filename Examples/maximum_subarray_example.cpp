// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Maximum_Subarray.cpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <vector>

int main()
{
    const std::vector<int> values{-5, 4, -1, 6, -9};
    const auto best = sirkit::maximum_subarray_sum(values);
    assert(best && best->left == 1 && best->right == 4 && best->sum == 9);
    const std::array grid{std::array{-9, -8, -7}, std::array{-6, 1, 2}};
    const auto rect = sirkit::maximum_submatrix_sum(2, 3, [&](std::size_t row, std::size_t column)
                                                    {
                                                        return grid[row][column];
                                                    });
    assert(rect && rect->top == 1 && rect->bottom == 2 && rect->left == 1 && rect->right == 3 && rect->sum == 3);
}
