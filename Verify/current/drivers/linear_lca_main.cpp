// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{}, q{};
    verify_in >> n >> q;
    std::vector<int> parents(n, -1);
    for (int v = 1; v < n; ++v)
    {
        verify_in >> parents[v];
    }
    const sirkit::LinearLCA lca(parents);
    for (int i = 0; i < q; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        verify_out << lca.get(u, v) << '\n';
    }
}
