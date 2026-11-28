// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<std::pair<int, int>> edges(n - 1);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    sirkit::DynamicSteinerSubtree tree(n, edges);
    for (int v = 0; v < n; ++v)
    {
        int marked{};
        verify_in >> marked;
        if (marked)
        {
            tree.set_marked(v);
        }
    }
    int queries{};
    verify_in >> queries;
    for (int q = 0; q < queries; ++q)
    {
        int type{}, v{};
        verify_in >> type >> v;
        --v;
        if (type == 1)
        {
            tree.toggle(v);
        }
        else
        {
            int subtree{};
            verify_in >> subtree;
            verify_out << tree.steiner_vertices(subtree - 1, v) << '\n';
        }
    }
}
