// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    const auto read_pair = [](std::pair<int, int> &edge)
    {
        auto &[u, v] = edge;
        verify_in >> u >> v;
        --u;
        --v;
    };
    std::vector<std::pair<int, int>> edges(n - 1), walks(m);
    for (auto &edge : edges)
    {
        read_pair(edge);
    }
    for (auto &walk : walks)
    {
        read_pair(walk);
    }
    const auto result = sirkit::orient_tree_walks(n, edges, walks);
    verify_out << result.covered_directions << '\n';
    for (std::size_t i = 0; i < walks.size(); ++i)
    {
        auto [u, v] = walks[i];
        if (result.reversed[i])
        {
            std::swap(u, v);
        }
        verify_out << u + 1 << ' ' << v + 1 << '\n';
    }
}
