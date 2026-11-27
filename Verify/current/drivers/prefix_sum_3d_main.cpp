// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>

int main()
{
    std::size_t n{};
    verify_in >> n;
    CumulativeSum3D<int> sums(n, n, n);
    for (std::size_t x = 0; x < n; ++x)
    {
        for (std::size_t y = 0; y < n; ++y)
        {
            for (std::size_t z = 0; z < n; ++z)
            {
                int value{};
                verify_in >> value;
                sums.add(x, y, z, value);
            }
        }
    }
    sums.build();
    std::size_t q{};
    verify_in >> q;
    for (std::size_t i = 0; i < q; ++i)
    {
        std::size_t lx{}, rx{}, ly{}, ry{}, lz{}, rz{};
        verify_in >> lx >> rx >> ly >> ry >> lz >> rz;
        verify_out << sums.query(lx - 1, ly - 1, lz - 1, rx, ry, rz) << '\n';
    }
}
