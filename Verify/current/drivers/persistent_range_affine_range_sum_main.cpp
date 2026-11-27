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
    PersistentAffineRangeSum<> tree(values);
    std::vector<PersistentAffineRangeSum<>::Root> roots(q + 1);
    roots[0] = tree.initial_root();
    for (int i = 1; i <= q; ++i)
    {
        int type = 0, k = 0, left = 0, right = 0;
        verify_in >> type >> k;
        if (type == 1)
        {
            int src = 0;
            verify_in >> src >> left >> right;
            roots[i] = tree.range_copy(roots[k + 1], roots[src + 1], left, right);
        }
        else
        {
            verify_in >> left >> right;
            if (type == 0)
            {
                std::int64_t a = 0, b = 0;
                verify_in >> a >> b;
                roots[i] = tree.range_affine(roots[k + 1], left, right, a, b);
            }
            else
            {
                verify_out << tree.range_sum(roots[k + 1], left, right) << '\n';
            }
        }
    }
}
