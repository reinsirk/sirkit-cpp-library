// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    std::uint64_t length = 0;
    int q = 0;
    verify_in >> length >> q;
    SparseAffineRangeSum<> data(length);
    while (q-- > 0)
    {
        int type = 0;
        std::uint64_t left = 0, right = 0;
        verify_in >> type >> left >> right;
        if (type == 0)
        {
            std::uint32_t a = 0, b = 0;
            verify_in >> a >> b;
            data.range_affine(left, right, a, b);
        }
        else
        {
            verify_out << data.range_sum(left, right) << '\n';
        }
    }
}
