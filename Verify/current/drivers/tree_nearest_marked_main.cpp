// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int n{};
    verify_in >> n;
    sirkit::Graph graph(n);
    for (int i = 1; i < n; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        --u;
        --v;
        graph[u].push_back(v);
        graph[v].push_back(u);
    }
    sirkit::TreeNearestMarked tree(graph);
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
            verify_out << tree.nearest(v)->distance << '\n';
        }
    }
}
