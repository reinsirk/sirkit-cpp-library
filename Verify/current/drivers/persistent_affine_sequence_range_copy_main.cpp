// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto &x : values)
    {
        verify_in >> x;
    }
    auto rope = sirkit::PersistentAffineSequence<std::int64_t>(values);
    for (int i = 0; i < q; ++i)
    {
        int type = 0, left = 0, right = 0;
        verify_in >> type >> left >> right;
        --left;
        if (type == 1)
        {
            std::int64_t add = 0;
            verify_in >> add;
            rope = rope.affine(left, right, 1, add);
        }
        else if (type == 2)
        {
            int from = 0, to = 0;
            verify_in >> from >> to;
            rope = rope.replace(left, right, rope.slice(from - 1, to));
        }
        else
        {
            verify_out << rope.sum(left, right) << '\n';
        }
    }
}
