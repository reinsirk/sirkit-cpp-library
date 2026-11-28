// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{}, colors{};
    verify_in >> n >> colors;
    std::vector<sirkit::TreeColorEdge> edges(n - 1);
    for (auto &[u, v, color] : edges)
    {
        verify_in >> u >> v >> color;
        --u;
        --v;
    }
    verify_out << sirkit::tree_edge_color_path_counts(n, edges).two_colors << '\n';
}
