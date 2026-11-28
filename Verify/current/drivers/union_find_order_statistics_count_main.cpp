// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n, q;
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &key : values)
    {
        verify_in >> key;
    }
    sirkit::UnionFindOrderStatistics tree(values);
    while (q-- > 0)
    {
        int type, a, b;
        verify_in >> type >> a >> b;
        if (type == 1)
        {
            tree.merge(a - 1, b - 1);
        }
        else
        {
            verify_out << tree.count(a - 1, b) << '\n';
        }
    }
}
