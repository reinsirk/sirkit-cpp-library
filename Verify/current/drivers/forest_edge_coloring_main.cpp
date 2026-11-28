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
    const auto answer = sirkit::ForestColoring(n, edges).edge_coloring();
    verify_out << answer.color_count << '\n';
    for (const int color : answer.colors)
    {
        verify_out << color + 1 << '\n';
    }
}
