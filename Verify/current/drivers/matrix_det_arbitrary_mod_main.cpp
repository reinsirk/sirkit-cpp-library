// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n;
    std::uint32_t mod;
    verify_in >> n >> mod;
    std::vector<std::vector<std::int64_t>> matrix(n, std::vector<std::int64_t>(n));
    for (auto &row : matrix)
    {
        for (auto &value : row)
        {
            verify_in >> value;
        }
    }
    verify_out << determinant_mod(matrix, mod) << '\n';
}
