// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Directed_Return_Distances.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Arc = sirkit::ReturnDistanceArc;
using Entry = std::optional<std::uint64_t>;

void check(int n, const std::vector<Arc> &edges)
{
    // Floyd–Warshall retaining only nonempty walks, so the diagonal starts at infinity.
    constexpr std::uint64_t kInf = 1'000'000;
    std::vector<std::vector<std::uint64_t>> distance(n, std::vector<std::uint64_t>(n, kInf));
    for (const auto &[u, v, weight] : edges)
    {
        distance[u][v] = std::min(distance[u][v], weight);
    }
    for (int k = 0; k < n; ++k)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                distance[u][v] = std::min(distance[u][v], distance[u][k] + distance[k][v]);
            }
        }
    }
    const auto actual = sirkit::directed_return_distances(n, edges);
    assert(actual.size() == static_cast<std::size_t>(n));
    for (int u = 0; u < n; ++u)
    {
        assert(actual[u] == (distance[u][u] == kInf ? std::nullopt : Entry(distance[u][u])));
    }
}

template <typename Error, typename Action>
void expect_error(Action action)
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
    check(0, {});
    check(1, {});
    check(1, {{0, 0, 9}, {0, 0, 0}, {0, 0, 2}});
    check(6, {{0, 1, 0}, {1, 2, 5}, {2, 0, 2}, {2, 4, 1}, {3, 5, 3}, {5, 5, 1}});
    check(4, {{0, 1, 9}, {0, 1, 1}, {0, 1, 20}, {1, 2, 1}, {2, 3, 50}, {3, 0, 50}});
    check(4, {{0, 1, 0}, {1, 2, 0}, {2, 0, 0}, {2, 3, 1}, {3, 1, 6}, {1, 0, 9}});
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const std::vector<Arc> wide{{0, 1, kMax - 1}, {1, 0, 1}};
    const std::vector<Arc> wide_ring_overflow{{0, 1, kMax}, {1, 2, kMax}, {2, 0, 1}};
    const std::vector<Arc> wide_branch_overflow{{0, 1, kMax}, {0, 0, 1}, {1, 2, 1}, {2, 0, 0}};
    const std::vector<Arc> wide_branch_exact{{0, 1, kMax - 1}, {0, 1, kMax}, {1, 0, 1}};
    assert((sirkit::directed_return_distances(2, wide) == std::vector<Entry>{kMax, kMax}));
    assert((sirkit::directed_return_distances(3, wide_ring_overflow) == std::vector<Entry>(3)));
    assert((sirkit::directed_return_distances(3, wide_branch_overflow) == std::vector<Entry>{1, std::nullopt, std::nullopt}));
    assert((sirkit::directed_return_distances(2, wide_branch_exact) == std::vector<Entry>{kMax, kMax}));

    std::mt19937 random(19192262);
    for (int trial = 0; trial < 950; ++trial)
    {
        const int n = static_cast<int>(random() % 13) + 1;
        const auto m = random() % (n * n + 1);
        std::vector<Arc> edges;
        for (unsigned i = 0; i < m; ++i)
        {
            edges.push_back({static_cast<int>(random() % n), static_cast<int>(random() % n), random() % 40});
        }
        check(n, edges);
    }

    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::directed_return_distances(-1, {});
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int bad : {-1, 2})
    {
        const std::vector<Arc> from{{bad, 0, 1}}, to{{0, bad, 1}};
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::directed_return_distances(2, from);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::directed_return_distances(2, to);
                                        });
    }
#endif
}
