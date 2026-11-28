// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n, q;
    verify_in >> n;
    std::vector<std::pair<int, int>> edges(n - 1);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    sirkit::TreeMarkedDiameter tree(n, edges, true);
    verify_in >> q;
    while (q-- > 0)
    {
        int v;
        verify_in >> v;
        tree.toggle(v - 1);
        verify_out << tree.diameter()->distance << '\n';
    }
}
