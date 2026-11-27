// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n, m;
    verify_in >> n >> m;
    std::vector<std::uint64_t> value(n);
    for (auto &x : value)
    {
        verify_in >> x;
    }
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
    }
    constexpr std::uint64_t kMod = 998244353;
    std::uint64_t sum = 0;
    enumerate_triangles(n, edges, [&](int u, int v, int w)
                        {
                            sum = (sum + value[u] * value[v] % kMod * value[w]) % kMod;
                        });
    verify_out << sum << '\n';
}
