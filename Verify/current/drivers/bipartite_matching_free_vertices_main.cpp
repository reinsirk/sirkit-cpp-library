// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <array>
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
    const auto coloring = sirkit::bipartite_coloring(n, edges);
    std::array<int, 2> sizes{};
    std::vector<int> labels(n);
    for (int v = 0; v < n; ++v)
    {
        labels[v] = sizes[coloring.color[v]]++;
    }
    for (auto &[u, v] : edges)
    {
        if (coloring.color[u] != 0)
        {
            std::swap(u, v);
        }
        u = labels[u];
        v = labels[v];
    }
    const auto analysis = sirkit::analyze_bipartite_matching(sizes[0], sizes[1], edges);
    const auto answer = std::ranges::count(analysis.left_can_be_free, true) +
                        std::ranges::count(analysis.right_can_be_free, true);
    verify_out << answer << '\n';
}
