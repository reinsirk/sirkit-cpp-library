// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &edge : edges)
    {
        verify_in >> edge.first;
        --edge.first;
    }
    for (auto &edge : edges)
    {
        verify_in >> edge.second;
        --edge.second;
    }
    verify_out << (sirkit::bipartite_coloring(n, edges).is_bipartite() ? "Yes\n" : "No\n");
}
