// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{};
    std::size_t k{};
    verify_in >> n >> m >> k;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const auto orders = sirkit::enumerate_topological_orders(n, edges, k);
    if (orders.size() < k)
    {
        verify_out << "-1\n";
        return 0;
    }
    for (const auto &order : orders)
    {
        for (int i = 0; i < n; ++i)
        {
            verify_out << order[i] + 1 << (i + 1 == n ? '\n' : ' ');
        }
    }
}
