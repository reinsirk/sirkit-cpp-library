// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/General_Weighted_Matching.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::GeneralWeightedMatchingEdge;

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

void compare(int n, std::span<const Edge> edges)
{
    std::vector<std::vector<std::int64_t>> weights(n, std::vector<std::int64_t>(n));
    for (const auto &[u, v, weight] : edges)
    {
        if (u != v)
        {
            weights[u][v] = weights[v][u] = std::max(weights[u][v], weight);
        }
    }
    std::vector<std::int64_t> best(1U << n);
    for (unsigned available = 1; available < best.size(); ++available)
    {
        const int first = std::countr_zero(available);
        const unsigned other = available & (available - 1);
        best[available] = best[other];
        for (auto remaining = other; remaining != 0; remaining &= remaining - 1)
        {
            const int second = std::countr_zero(remaining);
            const auto pair = best[other ^ (1U << second)] + weights[first][second];
            best[available] = std::max(best[available], pair);
        }
    }

    const auto answer = sirkit::maximum_weight_matching(n, edges);
    assert(answer.weight == best.back());
    std::vector<bool> used(n);
    std::int64_t total = 0;
    for (const auto &[u, v] : answer.edges)
    {
        assert(0 <= u && u < n && 0 <= v && v < n && u != v);
        assert(!used[u] && !used[v] && weights[u][v] > 0);
        used[u] = used[v] = true;
        total += weights[u][v];
    }
    assert(answer.weight == total);
}

void check_limits()
{
    constexpr auto kMaxWeight = std::numeric_limits<std::int64_t>::max() / 4;
    compare(0, {});
    compare(4, {});
    const std::array mixed{Edge{0, 0, std::numeric_limits<std::int64_t>::max()},
                           Edge{0, 1, 0}, Edge{1, 2, -7}, Edge{0, 1, 7}, Edge{1, 0, 6},
                           Edge{2, 1, 4}, Edge{2, 3, kMaxWeight}, Edge{1, 3, kMaxWeight - 4}};
    compare(4, mixed);
    must_throw<std::length_error>([] { static_cast<void>(sirkit::maximum_weight_matching(-1, {})); });
    must_throw<std::length_error>([] {
        const int too_many = std::numeric_limits<int>::max() / 2 + 1;
        static_cast<void>(sirkit::maximum_weight_matching(too_many, {}));
    });
    must_throw<std::overflow_error>([] {
        const std::array edges{Edge{0, 1, kMaxWeight + 1}};
        static_cast<void>(sirkit::maximum_weight_matching(2, edges));
    });
    must_throw<std::overflow_error>([] {
        const std::array edges{Edge{0, 1, kMaxWeight}, Edge{2, 3, kMaxWeight},
                               Edge{4, 5, kMaxWeight}, Edge{6, 7, kMaxWeight}, Edge{8, 9, kMaxWeight}};
        static_cast<void>(sirkit::maximum_weight_matching(10, edges));
    });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[u, v] : {std::pair{-1, 1}, std::pair{2, 0}, std::pair{0, -1}, std::pair{0, 2}})
    {
        must_throw<std::out_of_range>([&] {
            const std::array edges{Edge{u, v, 1}};
            static_cast<void>(sirkit::maximum_weight_matching(2, edges));
        });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(20261128);
    constexpr std::array kWeightRange{3, 6, 35, 257};
    for (int n = 1; n <= 15; ++n)
    {
        for (int test = 0; test < 200; ++test)
        {
            std::vector<Edge> edges;
            for (int u = 0; u < n; ++u)
            {
                for (int v = u + 1; v < n; ++v)
                {
                    const auto weight = static_cast<std::int64_t>(rng() % kWeightRange[test % 4]);
                    if (rng() % 100 < static_cast<unsigned>(test % 110) && weight > 0)
                    {
                        edges.push_back({u, v, weight});
                    }
                }
            }
            compare(n, edges);
        }
    }
}

void check_dense()
{
    constexpr int kVertices = 500;
    std::vector<Edge> edges;
    edges.reserve(kVertices * (kVertices - 1) / 2);
    for (int u = 0; u < kVertices; ++u)
    {
        for (int v = u + 1; v < kVertices; ++v)
        {
            const auto weight = u / 2 == v / 2 ? 1'000'000 : 1 + (u * v % 991);
            edges.push_back({u, v, weight});
        }
    }
    const auto answer = sirkit::maximum_weight_matching(kVertices, edges);
    assert(answer.weight == static_cast<std::int64_t>(kVertices / 2) * 1'000'000);
    assert(answer.edges.size() == kVertices / 2);
    for (const auto &[u, v] : answer.edges)
    {
        assert(u / 2 == v / 2 && u != v);
    }
}
}

int main()
{
    check_limits();
    check_random();
    check_dense();
}
