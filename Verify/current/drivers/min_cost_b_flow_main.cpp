// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<std::int64_t> supply(n);
    for (auto &value : supply)
    {
        verify_in >> value;
    }
    std::vector<sirkit::BFlowEdge> edges(m);
    for (auto &edge : edges)
    {
        verify_in >> edge.from >> edge.to >> edge.lower >> edge.upper >> edge.cost;
    }
    const auto answer = sirkit::min_cost_b_flow(supply, edges);
    if (!answer)
    {
        verify_out << "infeasible\n";
        return 0;
    }

    // この問題では各積の絶対値が10^18以下、項数は1000以下。
    constexpr std::int64_t kBase = 1'000'000'000'000'000'000;
    std::int64_t high = 0, low = 0;
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        low += edges[i].cost * answer->flow[i];
        high += low / kBase;
        low %= kBase;
    }
    if (high > 0 && low < 0)
    {
        --high;
        low += kBase;
    }
    else if (high < 0 && low > 0)
    {
        ++high;
        low -= kBase;
    }
    if (high == 0)
    {
        verify_out << low << '\n';
    }
    else
    {
        verify_out << high;
        auto magnitude = low < 0 ? -low : low;
        std::array<char, 18> suffix;
        for (auto it = suffix.rbegin(); it != suffix.rend(); ++it)
        {
            *it = static_cast<char>('0' + magnitude % 10);
            magnitude /= 10;
        }
        for (const char digit : suffix)
        {
            verify_out << digit;
        }
        verify_out << '\n';
    }
    for (const auto p : answer->potential)
    {
        verify_out << p << '\n';
    }
    for (const auto flow : answer->flow)
    {
        verify_out << flow << '\n';
    }
}
