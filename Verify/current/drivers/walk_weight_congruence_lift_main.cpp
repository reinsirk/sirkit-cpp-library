// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::WalkCongruenceEdge> edges;
    edges.reserve(2 * static_cast<std::size_t>(m));
    for (int i = 0; i < m; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        edges.push_back({u, v, 1});
        edges.push_back({v, u, -1});
    }
    const sirkit::WalkWeightCongruence weights(n, edges);
    std::uint64_t answer = 0;
    for (const auto period : weights.periods())
    {
        if (period == 0)
        {
            verify_out << -1 << '\n';
            return 0;
        }
        answer += period;
    }
    verify_out << answer << '\n';
}
