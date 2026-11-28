// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t rows{}, columns{};
    verify_in >> rows >> columns;
    std::vector<std::uint32_t> coefficients(rows * columns);
    for (auto &coefficient : coefficients)
    {
        verify_in >> coefficient;
    }
    const auto answer = sirkit::bivariate_fps_inverse(coefficients, rows, columns);
    for (std::size_t row = 0; row < rows; ++row)
    {
        for (std::size_t column = 0; column < columns; ++column)
        {
            verify_out << answer[row * columns + column] << (column + 1 == columns ? '\n' : ' ');
        }
    }
}
