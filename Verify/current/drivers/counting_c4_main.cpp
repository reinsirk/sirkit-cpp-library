// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    int n;
    std::size_t m;
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
    }
    const auto count = count_four_cycles_per_edge(n, edges);
    for (std::size_t i = 0; i < m; ++i)
    {
        verify_out << count[i] << (i + 1 == m ? '\n' : ' ');
    }
}
