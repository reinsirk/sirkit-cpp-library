// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n{}, q{};
    verify_in >> n >> q;
    std::vector<std::pair<int, int>> changes(q);
    for (auto &[u, v] : changes)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    auto edges = changes;
    std::sort(edges.begin(), edges.end());
    edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
    sirkit::DynamicEdgeProducts<std::int64_t> products(n, edges);
    for (const auto &edge : changes)
    {
        const auto id = static_cast<std::size_t>(std::lower_bound(edges.begin(), edges.end(), edge) - edges.begin());
        const bool activate = !products.active(id);
        products.set_active(id, activate);
        const int delta = activate ? 1 : -1;
        products.add(edge.first, delta);
        products.add(edge.second, delta);
        verify_out << products.sum_edge_products() << '\n';
    }
}
