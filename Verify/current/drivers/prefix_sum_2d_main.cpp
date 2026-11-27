// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>

int main()
{
    std::size_t n{}, m{}, q{};
    verify_in >> n >> m >> q;
    CumulativeSum2D<int> sums(n, n);
    for (std::size_t i = 0; i < m; ++i)
    {
        std::size_t left{}, right{};
        verify_in >> left >> right;
        sums.add(left - 1, right - 1, 1);
    }
    sums.build();
    for (std::size_t i = 0; i < q; ++i)
    {
        std::size_t left{}, right{};
        verify_in >> left >> right;
        verify_out << sums.query(left - 1, left - 1, right, right) << '\n';
    }
}
