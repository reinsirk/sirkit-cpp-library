// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <numeric>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{}, q{};
    verify_in >> n >> m >> q;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    std::vector<int> initial(n);
    std::iota(initial.begin(), initial.end(), 1);
    sirkit::NeighborhoodAssignment<int> colors(initial, edges);
    while (q-- > 0)
    {
        int v{};
        verify_in >> v;
        --v;
        colors.assign_neighbors(v, colors.get(v));
    }
    for (int v = 0; v < n; ++v)
    {
        verify_out << colors.get(v) << (v + 1 == n ? '\n' : ' ');
    }
}
