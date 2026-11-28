// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n, q;
    verify_in >> n >> q;
    std::vector<std::int64_t> initial(n);
    for (auto &value : initial)
    {
        verify_in >> value;
    }
    sirkit::RangeFenwickTree<std::int64_t> tree(initial);
    while (q-- > 0)
    {
        int type, first, last;
        verify_in >> type >> first >> last;
        --first;
        if (type == 1)
        {
            std::int64_t delta;
            verify_in >> delta;
            tree.add(first, last, delta);
        }
        else
        {
            verify_out << tree.sum(first, last) << '\n';
        }
    }
}
