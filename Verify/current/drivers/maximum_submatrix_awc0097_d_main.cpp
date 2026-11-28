// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t rows{}, columns{};
    verify_in >> rows >> columns;
    std::vector<std::vector<std::int64_t>> grid(rows, std::vector<std::int64_t>(columns));
    for (auto &row : grid)
    {
        for (auto &entry : row)
        {
            verify_in >> entry;
        }
    }
    const auto answer = sirkit::maximum_submatrix_sum(rows, columns, [&](std::size_t row, std::size_t column)
                                                      {
                                                          return grid[row][column];
                                                      });
    verify_out << answer->sum << '\n';
}
