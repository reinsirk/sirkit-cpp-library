// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <string>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::string bits;
    bits.reserve(n);
    verify_in >> bits;
    sirkit::BinaryRangeStatistics tree(bits);
    for (int i = 0; i < q; ++i)
    {
        int type = 0, left = 0, right = 0;
        verify_in >> type >> left >> right;
        if (type == 1)
        {
            tree.flip(left - 1, right);
        }
        else
        {
            verify_out << tree.stats(left - 1, right).longest[1] << '\n';
        }
    }
}
