// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_INCREMENTAL_SCC_DEFINED
#include "../../../Graph/SCC.cpp"
#include "../../../DataStructure/union_find.cpp"
#include "../../../Graph/Incremental_SCC.cpp"
#endif

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

void check(int n, const std::vector<Edge> &edges)
{
    const auto events = sirkit::incremental_scc_merges(n, edges);
    assert(events.size() <= static_cast<std::size_t>(n == 0 ? 0 : n - 1));
    std::vector<int> groups(n);
    std::iota(groups.begin(), groups.end(), 0);
    std::vector<std::uint64_t> outgoing(n);
    for (int v = 0; v < n; ++v)
    {
        outgoing[v] = std::uint64_t{1} << v;
    }

    std::size_t next = 0;
    assert(events.empty() || events.front().time > 0);
    for (std::size_t time = 1; time <= edges.size(); ++time)
    {
        const auto &[from, to] = edges[time - 1];
        outgoing[from] |= std::uint64_t{1} << to;
        auto reach = outgoing;
        for (int via = 0; via < n; ++via)
        {
            for (int from = 0; from < n; ++from)
            {
                if ((reach[from] >> via) & 1)
                {
                    reach[from] |= reach[via];
                }
            }
        }

        while (next < events.size() && events[next].time == time)
        {
            const auto &[ignored, leader, absorbed] = events[next++];
            assert(leader >= 0 && leader < n && absorbed >= 0 && absorbed < n);
            assert(leader != absorbed && groups[leader] == leader && groups[absorbed] == absorbed);
            assert(((reach[leader] >> absorbed) & 1) && ((reach[absorbed] >> leader) & 1));
            for (auto &group : groups)
            {
                if (group == absorbed)
                {
                    group = leader;
                }
            }
        }
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                const bool same = ((reach[u] >> v) & 1) && ((reach[v] >> u) & 1);
                assert((groups[u] == groups[v]) == same);
            }
        }
    }
    assert(next == events.size());
}

template <class Exception>
void expect_error(int n, std::initializer_list<Edge> edges)
{
    try
    {
        static_cast<void>(sirkit::incremental_scc_merges(n, {edges.begin(), edges.size()}));
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
} // namespace

int main()
{
    check(0, {});
    check(4, {});
    check(2, {{0, 0}, {1, 1}, {0, 0}});
    check(4, {{0, 1}, {1, 2}, {1, 2}, {0, 2}, {2, 3}, {0, 0}});
    check(5, {{0, 1}, {1, 0}, {1, 2}, {2, 3}, {3, 2}, {3, 4}, {3, 0}, {4, 1}, {0, 1}, {4, 4}});
    check(3, {{0, 1}, {1, 0}, {0, 1}, {1, 1}, {2, 0}, {1, 2}, {2, 0}, {0, 2}});

    std::mt19937 random(826441);
    for (int n = 1; n <= 13; ++n)
    {
        for (int trial = 0; trial < 240; ++trial)
        {
            const auto m = random() % (7 * n + 1);
            std::vector<Edge> edges;
            for (std::size_t i = 0; i < m; ++i)
            {
                edges.emplace_back(static_cast<int>(random() % n), static_cast<int>(random() % n));
            }
            check(n, edges);
        }
    }

    expect_error<std::length_error>(-1, {});
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::out_of_range>(0, {{0, 0}});
    expect_error<std::out_of_range>(2, {{-1, 1}});
    expect_error<std::out_of_range>(2, {{2, 1}});
    expect_error<std::out_of_range>(2, {{0, -1}});
    expect_error<std::out_of_range>(2, {{0, 2}});
#endif

    constexpr int kSize = 42000;
    std::vector<Edge> cycle;
    for (int v = 1; v < kSize; ++v)
    {
        cycle.emplace_back(v - 1, v);
    }
    assert(sirkit::incremental_scc_merges(kSize, cycle).empty());
    cycle.emplace_back(kSize - 1, 0);
    const auto events = sirkit::incremental_scc_merges(kSize, cycle);
    assert(events.size() == kSize - 1);
    std::vector<int> component_size(kSize, 1);
    int last_leader = -1;
    for (const auto &[time, leader, absorbed] : events)
    {
        assert(time == kSize);
        assert(component_size[leader] > 0 && component_size[absorbed] > 0);
        component_size[leader] += component_size[absorbed];
        component_size[absorbed] = 0;
        last_leader = leader;
    }
    assert(component_size[last_leader] == kSize);
}
