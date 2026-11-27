// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<int> parent(n);
    parent[0] = -1;
    for (int v = 1; v < n; ++v)
    {
        verify_in >> parent[v];
    }
    std::vector<std::uint64_t> c(n), d(n);
    for (auto &value : c)
    {
        verify_in >> value;
    }
    for (auto &value : d)
    {
        verify_in >> value;
    }
    const auto best = minimum_inversion_topological_order(n, parent, c, d);
    verify_out << best.inversions << '\n';
    for (int i = 0; i < n; ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << best.order[i];
    }
    verify_out << '\n';
}
