// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

int main()
{
    int n, m;
    verify_in >> n >> m;
    std::vector<std::uint32_t> weight(n);
    for (auto &x : weight)
    {
        verify_in >> x;
    }
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[from, to] : edges)
    {
        verify_in >> from >> to;
    }

    constexpr std::uint64_t kMod = 998244353;
    std::uint64_t answer = 0;
    const auto visit = [&](std::span<const int> clique)
    {
        std::uint64_t product = 1;
        for (int v : clique)
        {
            product = product * weight[v] % kMod;
        }
        answer += product;
        if (answer >= kMod)
        {
            answer -= kMod;
        }
    };
    enumerate_cliques(n, edges, visit);
    verify_out << answer << '\n';
}
