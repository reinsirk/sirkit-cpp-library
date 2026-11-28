// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <atcoder/modint>

#include <cstddef>
#include <vector>

int main()
{
    using Mint = atcoder::static_modint<1'000'000'007>;
    std::size_t size{};
    verify_in >> size;
    std::vector<int> matrix(size * size);
    for (auto &entry : matrix)
    {
        verify_in >> entry;
    }
    const auto answer = sirkit::matrix_permanent<Mint>(size, [&](std::size_t row, std::size_t column)
                                                       {
                                                           return matrix[row * size + column];
                                                       });
    verify_out << answer.val() << '\n';
}
