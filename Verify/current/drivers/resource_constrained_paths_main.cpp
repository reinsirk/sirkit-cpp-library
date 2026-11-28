// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    std::uint64_t initial{};
    verify_in >> n >> m >> initial;
    std::vector<sirkit::ResourcePathArc> edges;
    edges.reserve(2 * m + n);
    for (int i = 0; i < m; ++i)
    {
        int u{}, v{};
        std::uint64_t fare{}, time{};
        verify_in >> u >> v >> fare >> time;
        --u;
        --v;
        edges.push_back({u, v, fare, 0, time});
        edges.push_back({v, u, fare, 0, time});
    }
    for (int v = 0; v < n; ++v)
    {
        std::uint64_t coins{}, time{};
        verify_in >> coins >> time;
        edges.push_back({v, v, 0, coins, time});
    }
    // 両替の間に通る単純路は最大 n - 1 区間分の運賃で足りる。
    const sirkit::ResourceConstrainedPaths paths(n, edges, 0, 50 * n, initial);
    for (int v = 1; v < n; ++v)
    {
        verify_out << paths.minimum_time(v).value() << '\n';
    }
}
