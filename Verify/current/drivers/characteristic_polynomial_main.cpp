// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n;
    verify_in >> n;
    std::vector<std::vector<std::int64_t>> matrix(n, std::vector<std::int64_t>(n));
    for (auto &row : matrix)
    {
        for (auto &value : row)
        {
            verify_in >> value;
        }
    }
    const auto polynomial = characteristic_polynomial(matrix);
    for (std::size_t i = 0; i < polynomial.size(); ++i)
    {
        verify_out << polynomial[i] << (i == n ? '\n' : ' ');
    }
}
