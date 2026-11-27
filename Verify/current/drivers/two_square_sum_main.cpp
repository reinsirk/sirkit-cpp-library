// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    int q;
    verify_in >> q;
    for (int i = 0; i < q; ++i)
    {
        std::uint64_t n;
        verify_in >> n;
        const auto pairs = two_square_sum(n);
        verify_out << pairs.size() << '\n';
        for (const auto &[x, y] : pairs)
        {
            verify_out << x << ' ' << y << '\n';
        }
    }
}
