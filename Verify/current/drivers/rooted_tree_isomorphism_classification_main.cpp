// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<int> parent(n, -1);
    for (int i = 1; i < n; ++i)
    {
        verify_in >> parent[i];
    }
    const auto answer = rooted_subtree_classes(parent);
    verify_out << answer.count << '\n';
    for (int i = 0; i < n; ++i)
    {
        verify_out << answer.class_id[i] << (i + 1 == n ? '\n' : ' ');
    }
}
