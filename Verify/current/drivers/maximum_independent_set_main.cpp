// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    int n, m;
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
    }

    const auto selected = maximum_independent_set(n, edges);
    verify_out << selected.size() << '\n';
    for (std::size_t i = 0; i < selected.size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << selected[i];
    }
    verify_out << '\n';
}
