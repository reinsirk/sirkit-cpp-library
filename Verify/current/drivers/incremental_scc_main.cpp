// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n, m;
    verify_in >> n >> m;
    std::vector<std::uint32_t> sum(n);
    for (auto &value : sum)
    {
        verify_in >> value;
    }
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[from, to] : edges)
    {
        verify_in >> from >> to;
    }
    const auto events = incremental_scc_merges(n, edges);

    constexpr std::uint32_t kMod = 998244353;
    std::uint32_t answer = 0;
    std::size_t next = 0;
    for (std::size_t time = 1; time <= edges.size(); ++time)
    {
        while (next < events.size() && events[next].time == time)
        {
            const auto &[ignored, leader, absorbed] = events[next++];
            const auto a = sum[leader], b = sum[absorbed];
            answer += static_cast<std::uint32_t>(std::uint64_t{a} * b % kMod);
            if (answer >= kMod)
            {
                answer -= kMod;
            }
            const auto total = a + b;
            sum[leader] = total >= kMod ? total - kMod : total;
        }
        verify_out << answer << '\n';
    }
}
