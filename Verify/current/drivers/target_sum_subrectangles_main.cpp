// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <string>
#include <vector>

int main()
{
    int tests{};
    verify_in >> tests;
    for (int i = 0; i < tests; ++i)
    {
        int height{}, width{};
        verify_in >> height >> width;
        std::vector<std::string> grid(static_cast<std::size_t>(height));
        for (auto &row : grid)
        {
            verify_in >> row;
        }
        verify_out << sirkit::count_subrectangles_with_sum(height, width, 0, [&](int row, int col)
        {
            return grid[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)] == '#' ? 1 : -1;
        }) << '\n';
    }
}
