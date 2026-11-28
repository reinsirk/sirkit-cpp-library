// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/MST_Sensitivity.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::SpanningTreeEdge;
using Weight = std::int64_t;

template<class Exception, class Operation>
void must_throw(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

struct ReferenceForest
{
    std::vector<int> representative;
    int groups;

    explicit ReferenceForest(int n) : representative(n), groups(n)
    {
        std::iota(representative.begin(), representative.end(), 0);
    }

    bool join(int u, int v)
    {
        const int before = representative[v], after = representative[u];
        if (before == after)
        {
            return false;
        }
        for (auto &label : representative)
        {
            if (label == before)
            {
                label = after;
            }
        }
        --groups;
        return true;
    }
};

void compare(int n, std::span<const Edge> edges)
{
    ReferenceForest full(n);
    for (const auto &[u, v, weight] : edges)
    {
        full.join(u, v);
    }
    const int rank = n - full.groups;
    std::vector<std::optional<Weight>> candidate(1U << edges.size());
    std::vector<std::optional<Weight>> include(edges.size()), exclude(edges.size());
    std::optional<Weight> best;
    for (unsigned mask = 0; mask < candidate.size(); ++mask)
    {
        if (std::popcount(mask) != rank)
        {
            continue;
        }
        ReferenceForest current(n);
        Weight sum = 0;
        for (auto set = mask; set != 0; set &= set - 1)
        {
            const auto &edge = edges[std::countr_zero(set)];
            current.join(edge.from, edge.to);
            sum += edge.weight;
        }
        if (current.groups != full.groups)
        {
            continue;
        }
        candidate[mask] = sum;
        if (!best || sum < *best)
        {
            best = sum;
        }
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            auto &slot = ((mask >> i) & 1) != 0 ? include[i] : exclude[i];
            if (!slot || sum < *slot)
            {
                slot = sum;
            }
        }
    }

    const auto result = sirkit::minimum_spanning_forest_sensitivity(n, edges);
    assert(result.forest.size() == static_cast<std::size_t>(rank));
    unsigned mask = 0;
    for (const auto id : result.forest)
    {
        mask |= 1U << id;
    }
    assert(best && candidate[mask] == best && std::popcount(mask) == rank);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const bool in = ((mask >> i) & 1) != 0;
        const auto goal = in ? exclude[i] : include[i];
        const auto swap = result.exchange[i];
        assert(goal.has_value() == swap.has_value());
        if (swap)
        {
            const bool exchange_in = ((mask >> *swap) & 1) != 0;
            assert(in != exchange_in);
            const auto changed = mask ^ (1U << i) ^ (1U << *swap);
            assert(candidate[changed] == goal);
        }
    }
}

void check_limits()
{
    compare(0, {});
    compare(1, {});
    const std::array only_loops{Edge{0, 0, -3}, Edge{1, 1, 4}};
    compare(4, only_loops);
    const std::array bridge{Edge{0, 1, 8}, Edge{2, 1, 3}, Edge{1, 1, 1}};
    compare(4, bridge);
    const std::array tied{Edge{0, 1, -3}, Edge{1, 2, -3}, Edge{0, 2, -2},
                          Edge{2, 0, -2}, Edge{3, 4, 4}};
    const auto chosen = sirkit::minimum_spanning_forest_sensitivity(6, tied);
    assert(chosen.exchange[0] == 2 && chosen.exchange[1] == 2);
    assert(chosen.exchange[2] == 0 && chosen.exchange[3] == 0);
    assert(!chosen.exchange[4]);
    compare(6, tied);
    const auto min = std::numeric_limits<Weight>::min(), max = std::numeric_limits<Weight>::max();
    const std::array extremes{Edge{0, 1, min}, Edge{1, 2, max}, Edge{2, 0, max}, Edge{0, 0, min}};
    const auto answer = sirkit::minimum_spanning_forest_sensitivity(3, extremes);
    assert((answer.forest == std::vector<std::size_t>{0, 1}));
    assert(answer.exchange[0] == 2 && answer.exchange[1] == 2 && answer.exchange[2] == 1);
    assert(!answer.exchange[3]);
    must_throw<std::length_error>([] { static_cast<void>(sirkit::minimum_spanning_forest_sensitivity(-1, {})); });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[u, v] : {std::pair{-1, 0}, std::pair{2, 0}, std::pair{0, -1}, std::pair{0, 2}})
    {
        must_throw<std::out_of_range>([&] {
            const std::array edges{Edge{u, v, 1}};
            static_cast<void>(sirkit::minimum_spanning_forest_sensitivity(2, edges));
        });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(202004);
    for (int test = 0; test < 650; ++test)
    {
        const int n = 1 + static_cast<int>(rng() % 8);
        const int m = static_cast<int>(rng() % 13);
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            const int u = static_cast<int>(rng() % n), v = static_cast<int>(rng() % n);
            const auto weight = static_cast<Weight>(rng() % (1 + test % 23)) - test % 7;
            edges.push_back({u, v, weight});
        }
        compare(n, edges);
    }
}
}

int main()
{
    check_limits();
    check_random();
}
