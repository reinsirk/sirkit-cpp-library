// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Walk_Weight_Congruence.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Edge = sirkit::WalkCongruenceEdge;

void check(int n, const std::vector<Edge> &edges, int modulus)
{
    std::vector graph(n, std::vector<std::pair<int, int>>{});
    std::vector reach(n, std::vector<bool>(n));
    for (int v = 0; v < n; ++v)
    {
        reach[v][v] = true;
    }
    for (const auto &[u, v, weight] : edges)
    {
        graph[u].emplace_back(v, weight);
        reach[u][v] = true;
    }
    for (int k = 0; k < n; ++k)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                reach[u][v] = reach[u][v] || (reach[u][k] && reach[k][v]);
            }
        }
    }
    const sirkit::WalkWeightCongruence actual(n, edges);
    std::vector<std::uint64_t> cycles(n);
    for (int start = 0; start < n; ++start)
    {
        auto visit = [&](auto &self, int v, int seen, std::int64_t total) -> void
        {
            for (const auto &[next, weight] : graph[v])
            {
                if (next == start)
                {
                    const auto sum = total + weight;
                    const auto magnitude = static_cast<std::uint64_t>(sum < 0 ? -sum : sum);
                    cycles[start] = std::gcd(cycles[start], magnitude);
                }
                else if ((seen & (1 << next)) == 0)
                {
                    self(self, next, seen | (1 << next), total + weight);
                }
            }
        };
        visit(visit, start, (1 << start), 0);
    }
    std::vector<std::uint64_t> component_periods;
    for (int start = 0; start < n; ++start)
    {
        std::uint64_t period = 0;
        int representative = start;
        for (int v = 0; v < n; ++v)
        {
            if (reach[start][v] && reach[v][start])
            {
                period = std::gcd(period, cycles[v]);
                representative = std::min(representative, v);
            }
        }
        assert(actual.period(start) == period);
        if (representative == start)
        {
            component_periods.push_back(period);
        }

        std::vector reachable(n, std::vector<bool>(modulus));
        reachable[start][0] = true;
        std::vector<std::pair<int, int>> pending{{start, 0}};
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const auto [v, residue] = pending[i];
            for (const auto &[next, weight] : graph[v])
            {
                const auto sum = static_cast<std::int64_t>(residue) + weight;
                const auto offset = (sum % modulus + modulus) % modulus;
                if (!reachable[next][offset])
                {
                    reachable[next][offset] = true;
                    pending.emplace_back(next, static_cast<int>(offset));
                }
            }
        }
        for (int v = 0; v < n; ++v)
        {
            const auto relation = actual.relation(start, v);
            assert(relation.has_value() == (reach[start][v] && reach[v][start]));
            if (!relation)
            {
                continue;
            }
            assert(relation->period == period);
            for (int residue = 0; residue < modulus; ++residue)
            {
                assert(relation->matches(residue, modulus) == reachable[v][residue]);
                assert(relation->matches(residue - modulus, modulus) == reachable[v][residue]);
            }
        }
    }
    std::vector reported(actual.periods().begin(), actual.periods().end());
    std::sort(component_periods.begin(), component_periods.end());
    std::sort(reported.begin(), reported.end());
    assert(component_periods == reported);
}

template <typename Error, typename Action>
void check_error(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}
} // namespace

int main()
{
    check(0, {}, 1);
    check(4, {}, 4);
    check(3, {{0, 1, -5}, {1, 2, -4}, {2, 0, 7}, {2, 2, 4}}, 12);
    check(5, {{0, 1, -3}, {1, 2, 5}, {2, 0, -2}, {2, 3, 7}, {3, 4, 2}, {4, 3, -2}}, 7);
    check(2, {{0, 1, 7}, {1, 0, 3}, {0, 1, 3}, {0, 0, 0}}, 6);
    check(3, {{2, 1, std::numeric_limits<std::int32_t>::min()}, {1, 0, 1},
              {0, 2, std::numeric_limits<std::int32_t>::max()}}, 11);

    constexpr auto kLargeModulus = std::numeric_limits<std::uint64_t>::max();
    constexpr auto kMinimum = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMaximum = std::numeric_limits<std::int64_t>::max();
    assert((sirkit::WalkWeightClass{kMinimum, 0}.matches(kMaximum, kLargeModulus)));
    assert((!sirkit::WalkWeightClass{kMinimum, 0}.matches(0, kLargeModulus)));
    assert((sirkit::WalkWeightClass{1, 3}.matches(kMaximum, kLargeModulus)));
    assert((sirkit::WalkWeightClass{-100, 2}.matches(-10, 5)));

    std::mt19937 random(2378051);
    for (int test = 0; test < 1000; ++test)
    {
        const int n = static_cast<int>(random() % 7) + 1;
        const int m = static_cast<int>(random() % 24);
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            edges.push_back({static_cast<int>(random() % n), static_cast<int>(random() % n),
                             static_cast<int>(random() % 21) - 10});
        }
        check(n, edges, 1 + static_cast<int>(random() % 11));
    }

    check_error<std::length_error>([] { sirkit::WalkWeightCongruence bad(-1, {}); });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto invalid : std::vector<Edge>{{-1, 0, 7}, {2, 0, 7}, {0, -1, 7}, {0, 2, 7}})
    {
        check_error<std::out_of_range>(
            [&] { sirkit::WalkWeightCongruence bad(2, std::span(&invalid, 1)); });
    }
    const sirkit::WalkWeightCongruence isolated(2, {});
    check_error<std::out_of_range>([&] { (void)isolated.relation(-1, 0); });
    check_error<std::out_of_range>([&] { (void)isolated.relation(0, 2); });
    check_error<std::out_of_range>([&] { (void)isolated.period(2); });
    check_error<std::invalid_argument>([] { (void)sirkit::WalkWeightClass{0, 2}.matches(0, 0); });
#endif
}
