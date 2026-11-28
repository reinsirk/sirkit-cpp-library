// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    sirkit::PersistentAffineSequence<std::int64_t> sequence(values);
    for (int i = 0; i < q; ++i)
    {
        int type = 0, left = 0, right = 0;
        verify_in >> type >> left >> right;
        if (type == 0)
        {
            sequence = sequence.reverse(left, right);
        }
        else
        {
            verify_out << sequence.sum(left, right) << '\n';
        }
    }
}
