// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <atcoder/modint>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int height{}, width{}, count{};
    verify_in >> height >> width >> count;
    std::vector<std::pair<int, int>> walls(static_cast<std::size_t>(count));
    for (auto &[row, col] : walls)
    {
        verify_in >> row >> col;
        --row;
        --col;
    }
    using Mint = atcoder::modint1000000007;
    sirkit::PrimeBinomial<Mint> coefficients(static_cast<std::int64_t>(height) + width - 2);
    const auto ways = sirkit::count_sparse_grid_paths(height, width, walls, [&](std::int64_t n, std::int64_t k)
    {
        return coefficients.COM(n, k);
    });
    verify_out << ways.val() << '\n';
}
