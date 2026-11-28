// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    std::size_t n{}, m{};
    verify_in >> n >> m;
    std::vector<std::vector<int>> graph(n);
    std::vector<int> neighbors;
    for (std::size_t i = 0; i < m; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        --u;
        --v;
        if (u == 0 || v == 0)
        {
            neighbors.push_back(u + v);
        }
        else
        {
            graph[static_cast<std::size_t>(u)].push_back(v);
            graph[static_cast<std::size_t>(v)].push_back(u);
        }
    }
    verify_out << (sirkit::find_short_simple_path(graph, 4, neighbors, neighbors) ? "YES\n" : "NO\n");
}
