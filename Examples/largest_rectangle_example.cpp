// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Largest_Rectangle.cpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>

int main()
{
    const std::array<std::uint64_t, 7> heights{2, 1, 4, 5, 1, 3, 3};
    const auto histogram = sirkit::largest_histogram_rectangle(heights);
    assert(histogram.left == 2 && histogram.right == 4 && histogram.height == 4 && histogram.area == 8);
    const std::array<std::string_view, 3> grid{".##.", "....", "...."};
    const auto area = sirkit::largest_grid_rectangle(3, 4, [&](std::size_t row, std::size_t column)
                                                     {
                                                         return grid[row][column] == '.';
                                                     });
    assert(area.top == 1 && area.bottom == 3 && area.left == 0 && area.right == 4 && area.area == 8);
}
