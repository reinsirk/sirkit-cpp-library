// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const auto result = sirkit::orient_with_outdegree_parity(n, edges);
    if (!result)
    {
        verify_out << -1 << '\n';
        return 0;
    }
    for (const auto &[from, to] : *result)
    {
        verify_out << from + 1 << ' ' << to + 1 << '\n';
    }
}
