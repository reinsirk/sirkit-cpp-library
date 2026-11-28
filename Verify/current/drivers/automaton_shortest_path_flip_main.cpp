// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    std::uint64_t flip{};
    verify_in >> n >> m >> flip;
    std::vector<sirkit::AutomatonPathEdge> edges;
    edges.reserve(static_cast<std::size_t>(2 * m + n));
    for (int i = 0; i < m; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        --u;
        --v;
        edges.push_back({u, v, 0, 1});
        edges.push_back({v, u, 1, 1});
    }
    for (int v = 0; v < n; ++v)
    {
        edges.push_back({v, v, 2, flip});
    }
    auto next = [](int mode, int label) { return label == 2 ? 1 - mode : (label == mode ? mode : -1); };
    auto accept = [](int) { return true; };
    const auto path = sirkit::automaton_shortest_path(n, edges, 0, n - 1, 2, 0, next, accept);
    verify_out << path.value().distance << '\n';
}
