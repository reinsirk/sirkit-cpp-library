// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <vector>

int main()
{
    int n{}, m{}, s{};
    verify_in >> n >> m >> s;
    std::vector<std::vector<int>> graph(n);
    for (int i = 0; i < m; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        graph[u].push_back(v);
    }
    const auto immediate = dominator_tree(graph, s);
    for (int i = 0; i < n; ++i)
    {
        verify_out << immediate[i] << (i + 1 == n ? '\n' : ' ');
    }
}
