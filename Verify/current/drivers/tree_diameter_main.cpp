// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<TreeDiameterEdge> edges(n - 1);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
    }

    const auto result = tree_diameter(n, edges);
    verify_out << result.length << ' ' << result.path.size() << '\n';
    for (std::size_t i = 0; i < result.path.size(); ++i)
    {
        verify_out << result.path[i] << (i + 1 == result.path.size() ? '\n' : ' ');
    }
}
