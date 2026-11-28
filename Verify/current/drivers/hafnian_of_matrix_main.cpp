// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include "Verify/current/common/matrix_field.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::vector<VerifyMatrixField>> matrix(n, std::vector<VerifyMatrixField>(n));
    for (auto &row : matrix)
    {
        for (auto &entry : row)
        {
            std::uint32_t residue{};
            verify_in >> residue;
            entry = residue;
        }
    }
    verify_out << sirkit::hafnian(matrix).get() << '\n';
}
