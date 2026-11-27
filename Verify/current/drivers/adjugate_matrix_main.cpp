// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <atcoder/modint>
#include <cstddef>
#include <cstdint>

int main()
{
    using Mint = atcoder::modint998244353;
    std::size_t n;
    verify_in >> n;
    Matrix<Mint> matrix(n, n);
    for (std::size_t i = 0; i < n; ++i)
    {
        for (auto &entry : matrix.row(i))
        {
            unsigned value;
            verify_in >> value;
            entry = Mint::raw(value);
        }
    }
    const auto adj = MatrixAdjugate(matrix);
    for (std::size_t i = 0; i < n; ++i)
    {
        for (std::size_t j = 0; j < n; ++j)
        {
            verify_out << adj(i, j).val() << (j + 1 == n ? '\n' : ' ');
        }
    }
}
