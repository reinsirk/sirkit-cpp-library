// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Maximum_Independent_Set.cpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

bool independent(std::uint64_t mask, const std::vector<Edge> &edges)
{
    for (const auto &[u, v] : edges)
    {
        const auto endpoints = (std::uint64_t{1} << u) | (std::uint64_t{1} << v);
        if ((mask & endpoints) == endpoints)
        {
            return false;
        }
    }
    return true;
}

std::uint64_t to_mask(int n, const std::vector<int> &vertices)
{
    std::uint64_t mask = 0;
    int previous = -1;
    for (int v : vertices)
    {
        assert(v > previous && v < n);
        mask |= std::uint64_t{1} << v;
        previous = v;
    }
    return mask;
}

void check_small(const std::vector<std::int64_t> &weights, const std::vector<Edge> &edges)
{
    const auto n = static_cast<int>(weights.size());
    const auto full = std::uint64_t{1} << n;
    int best_count = 0;
    std::int64_t best_weight = 0;
    for (std::uint64_t subset = 0; subset < full; ++subset)
    {
        if (!independent(subset, edges))
        {
            continue;
        }
        best_count = std::max(best_count, std::popcount(subset));
        std::int64_t weight = 0;
        for (auto selected = subset; selected != 0; selected &= selected - 1)
        {
            weight += weights[std::countr_zero(selected)];
        }
        best_weight = std::max(best_weight, weight);
    }

    const auto maximum = sirkit::maximum_independent_set(n, edges);
    const auto mask = to_mask(n, maximum);
    assert(independent(mask, edges));
    assert(std::popcount(mask) == best_count);

    const auto weighted = sirkit::maximum_weight_independent_set(weights, edges);
    assert(independent(to_mask(n, weighted.vertices), edges));
    assert(weighted.weight == best_weight);
    std::int64_t restored_weight = 0;
    for (int v : weighted.vertices)
    {
        assert(weights[v] > 0);
        restored_weight += weights[v];
    }
    assert(restored_weight == best_weight);
}

template <class Exception>
void expect_error(const auto &operation)
{
    try
    {
        operation();
        assert(false);
    }
    catch (const Exception &)
    {
    }
}

void check_weighted_exception(std::initializer_list<std::int64_t> weights, const std::vector<Edge> &edges)
{
    expect_error<std::overflow_error>([&]
                                      {
                                          static_cast<void>(sirkit::maximum_weight_independent_set({weights.begin(), weights.size()}, edges));
                                      });
}
} // namespace

int main()
{
    check_small({}, {});
    check_small({1, 0, -3, 5}, {{0, 1}, {1, 0}, {0, 1}, {3, 3}});
    check_small({2, 2, 2, 2, 2, 2}, {{0, 3}, {1, 4}, {2, 5}, {3, 4}});
    check_small({1, 1, 1, 1}, {{0, 1}, {2, 3}});
    std::mt19937 random(615023);
    for (int n = 1; n <= 11; ++n)
    {
        for (int trial = 0; trial <= 145; ++trial)
        {
            const auto density = random() % 101;
            std::vector<Edge> edges;
            std::vector<std::int64_t> weights(n);
            for (int v = 0; v < n; ++v)
            {
                weights[v] = static_cast<std::int64_t>(random() % 37) - 12;
                for (int u = 0; u < v; ++u)
                {
                    if (random() % 100 < density)
                    {
                        edges.emplace_back(u, v);
                        if (random() % 13 == 0)
                        {
                            edges.emplace_back(v, u);
                        }
                    }
                }
                if (random() % 17 == 0)
                {
                    edges.emplace_back(v, v);
                }
            }
            check_small(weights, edges);
        }
    }

    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    const std::vector<std::int64_t> boundary{kMax, kMin, 0, 1};
    const std::vector<Edge> conflict{{0, 3}};
    const auto maximum = sirkit::maximum_weight_independent_set(boundary, conflict);
    assert(maximum.weight == kMax && maximum.vertices == std::vector<int>{0});

    check_weighted_exception({kMax, 1}, {});
    check_weighted_exception({kMax, kMax}, {});
    check_weighted_exception({kMax, 1, 1, 1}, {{0, 2}, {1, 3}});
    check_weighted_exception({1, 1, kMax, 1}, {{0, 2}, {1, 3}});

    expect_error<std::length_error>([]
                                    {
                                        static_cast<void>(sirkit::maximum_independent_set(-1, {}));
                                    });
    expect_error<std::length_error>([]
                                    {
                                        static_cast<void>(sirkit::maximum_independent_set(47, {}));
                                    });
    expect_error<std::length_error>([]
                                    {
                                        static_cast<void>(sirkit::maximum_weight_independent_set(std::vector<std::int64_t>(47), {}));
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &edge : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            static_cast<void>(sirkit::maximum_independent_set(2, {&edge, 1}));
                                        });
    }
#endif

    // 左右にそれぞれ23頂点、片側内部の辺はないため、片側を全探索する。
    constexpr int kHalf = 23;
    std::vector<Edge> matching;
    std::vector<std::int64_t> weights(2 * kHalf, 1);
    std::vector<int> expected;
    for (int v = 0; v < kHalf; ++v)
    {
        matching.emplace_back(v, v + kHalf);
        weights[v + kHalf] = 2;
        expected.push_back(v + kHalf);
    }
    const auto count_optimum = sirkit::maximum_independent_set(2 * kHalf, matching);
    assert(count_optimum.size() == kHalf && independent(to_mask(2 * kHalf, count_optimum), matching));
    const auto weight_optimum = sirkit::maximum_weight_independent_set(weights, matching);
    assert(weight_optimum.weight == 2 * kHalf && weight_optimum.vertices == expected);
}
