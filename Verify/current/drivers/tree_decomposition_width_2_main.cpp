// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    char p, t, w;
    int n, m;
    verify_in >> p >> t >> w >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }

    const auto result = tree_decomposition_width_two(n, edges);
    if (!result)
    {
        verify_out << "-1\n";
        return 0;
    }

    verify_out << "s td " << result->bags.size() << ' ' << result->width << ' ' << n << '\n';
    for (int i = 0; i < n; ++i)
    {
        const auto &bag = result->bags[i];
        verify_out << "b " << i + 1;
        for (int j = 0; j < bag.size; ++j)
        {
            verify_out << ' ' << bag.vertices[j] + 1;
        }
        verify_out << '\n';
    }
    for (const auto &[u, v] : result->tree_edges)
    {
        verify_out << u + 1 << ' ' << v + 1 << '\n';
    }
}
