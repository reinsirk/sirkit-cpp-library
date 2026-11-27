// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Minimum_Diameter_Spanning_Tree.cpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <numeric>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::MinimumDiameterTreeEdge;

std::optional<std::int64_t> diameter_of(
    int n, const std::vector<Edge> &edges, const std::vector<std::size_t> &selected)
{
    if (selected.size() != static_cast<std::size_t>(n - 1))
    {
        return std::nullopt;
    }
    std::vector<std::vector<std::pair<int, std::int64_t>>> graph(n);
    for (auto id : selected)
    {
        if (id >= edges.size())
        {
            return std::nullopt;
        }
        const auto &[u, v, weight] = edges[id];
        graph[u].emplace_back(v, weight);
        graph[v].emplace_back(u, weight);
    }
    std::int64_t diameter = 0;
    for (int source = 0; source < n; ++source)
    {
        std::vector<std::int64_t> distance(n, -1);
        distance[source] = 0;
        std::vector<int> queue{source};
        for (std::size_t i = 0; i < queue.size(); ++i)
        {
            const int v = queue[i];
            for (const auto &[to, weight] : graph[v])
            {
                if (distance[to] < 0)
                {
                    distance[to] = distance[v] + weight;
                    diameter = std::max(diameter, distance[to]);
                    queue.push_back(to);
                }
            }
        }
        if (queue.size() != static_cast<std::size_t>(n))
        {
            return std::nullopt;
        }
    }
    return diameter;
}

void check_small(int n, const std::vector<Edge> &edges)
{
    std::optional<std::int64_t> expected;
    const unsigned limit = 1U << edges.size();
    for (unsigned subset = 0; subset < limit; ++subset)
    {
        if (std::popcount(subset) != n - 1)
        {
            continue;
        }
        std::vector<std::size_t> selected;
        for (auto left = subset; left != 0; left &= left - 1)
        {
            selected.push_back(std::countr_zero(left));
        }
        const auto diameter = diameter_of(n, edges, selected);
        if (diameter && (!expected || *diameter < *expected))
        {
            expected = diameter;
        }
    }
    const auto result = sirkit::minimum_diameter_spanning_tree(n, edges);
    assert(result.has_value() == expected.has_value());
    if (result)
    {
        assert(result->diameter == *expected);
        assert(diameter_of(n, edges, result->edge_ids) == expected);
    }
}

template <class Exception>
void expect_error(int n, std::initializer_list<Edge> edges)
{
    try
    {
        static_cast<void>(sirkit::minimum_diameter_spanning_tree(n, {edges.begin(), edges.size()}));
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
} // namespace

int main()
{
    using sirkit::minimum_diameter_spanning_tree;
    assert(!minimum_diameter_spanning_tree(0, {}));
    check_small(1, {});
    check_small(1, {{0, 0, 6}});
    check_small(3, {{0, 1, 0}});
    check_small(2, {{0, 1, 7}, {1, 0, 5}, {0, 0, 1}});
    check_small(4, {{0, 1, 9}, {1, 2, 0}, {2, 3, 1}, {3, 0, 2}, {1, 3, 0}});
    check_small(4, {{0, 1, 3}, {0, 2, 3}, {0, 3, 2}, {1, 2, 4}, {2, 3, 5}});
    check_small(5, {{0, 1, 1}, {1, 2, 0}, {2, 3, 0}, {3, 4, 1}, {1, 3, 3}});
    check_small(3, {{0, 1, 0}, {1, 2, 0}, {0, 2, 0}});

    std::mt19937 random(91002);
    for (int n = 1; n <= 7; ++n)
    {
        for (int trial = 0; trial < 270; ++trial)
        {
            const auto m = random() % 13;
            const auto weight_limit = static_cast<std::int64_t>(2 + random() % 53);
            std::vector<Edge> edges;
            for (std::size_t i = 0; i < m; ++i)
            {
                edges.push_back({static_cast<int>(random() % n), static_cast<int>(random() % n),
                                 static_cast<std::int64_t>(random() % weight_limit)});
            }
            check_small(n, edges);
        }
    }

    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    const std::vector<Edge> exact_max{{0, 1, kMax}};
    const auto maximum = minimum_diameter_spanning_tree(2, exact_max);
    assert(maximum && maximum->diameter == kMax && maximum->edge_ids == std::vector<std::size_t>{0});
    const std::vector<Edge> dominated{{0, 1, 1}, {0, 1, kMax}, {1, 2, 2}, {0, 2, kMax}};
    const auto small = minimum_diameter_spanning_tree(3, dominated);
    assert(small && small->diameter == 3);
    expect_error<std::overflow_error>(3, {{0, 1, kMax}, {1, 2, 1}});
    expect_error<std::overflow_error>(3, {{0, 1, kMax}, {1, 2, kMax}, {2, 0, kMax}});
    const std::vector<Edge> both_missing_and_wide{{0, 1, kMax}, {1, 2, kMax}};
    assert(!minimum_diameter_spanning_tree(4, both_missing_and_wide));
    expect_error<std::length_error>(-1, {});
    expect_error<std::length_error>(std::numeric_limits<int>::max(), {});
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::out_of_range>(2, {{-1, 0, 1}});
    expect_error<std::out_of_range>(2, {{2, 0, 1}});
    expect_error<std::out_of_range>(2, {{0, -1, 1}});
    expect_error<std::out_of_range>(2, {{0, 2, 1}});
    expect_error<std::invalid_argument>(2, {{0, 1, -1}});
#endif

    constexpr int kSize = 260;
    constexpr std::int64_t kWeight = 999999999;
    std::vector<Edge> cycle;
    for (int v = 0; v < kSize; ++v)
    {
        cycle.push_back({v, (v + 1) % kSize, kWeight});
    }
    const auto widest = minimum_diameter_spanning_tree(kSize, cycle);
    assert(widest && widest->diameter == (kSize - 1) * kWeight);
    assert(diameter_of(kSize, cycle, widest->edge_ids) == widest->diameter);
}
