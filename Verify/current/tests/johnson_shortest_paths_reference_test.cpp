// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Johnson_Shortest_Paths.cpp"

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
using Edge = sirkit::JohnsonEdge;

void check(int n, const std::vector<Edge> &edges)
{
    using Distance = std::optional<std::int64_t>;
    std::vector via(n, std::vector<Distance>(n));
    for (int v = 0; v < n; ++v)
    {
        via[v][v] = 0;
    }
    auto relax = [&](int u, int v, std::int64_t weight)
    {
        if (!via[u][v] || weight < *via[u][v])
        {
            via[u][v] = weight;
        }
    };
    for (const auto &[u, v, weight] : edges)
    {
        relax(u, v, weight);
    }
    for (int k = 0; k < n; ++k)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                if (via[u][k] && via[k][v])
                {
                    relax(u, v, *via[u][k] + *via[k][v]);
                }
            }
        }
    }
    bool has_negative_cycle = false;
    for (int v = 0; v < n; ++v)
    {
        has_negative_cycle = has_negative_cycle || *via[v][v] < 0;
    }
    int next = 0;
    auto verify_row = [&](int source, std::span<const Distance> distance)
    {
        assert(!has_negative_cycle);
        assert(source == next++);
        assert(std::ranges::equal(distance, via[source]));
    };
    const bool ok = sirkit::for_each_johnson_distance_row(n, edges, verify_row);
    assert(ok == !has_negative_cycle);
    assert(next == (ok ? n : 0));
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
    check(0, {});
    check(5, {});
    check(4, {{0, 1, 3}, {0, 2, 1}, {2, 1, 1}, {1, 3, 11}});
    check(5, {{0, 1, 3}, {0, 1, -1}, {2, 3, -10}, {3, 4, 5}, {4, 2, 3}});
    check(5, {{0, 1, 3}, {0, 1, -1}, {2, 3, -10}, {3, 4, 5}, {4, 2, 5}});
    constexpr auto kCap = std::numeric_limits<std::int64_t>::max() / 4 / 4;
    check(4, {{0, 1, -kCap}, {1, 2, -kCap}, {2, 3, -kCap}, {0, 3, kCap}});
    check(3, {{0, 1, -1000000000000}, {1, 2, 1000000000000}, {0, 2, 1}});
    std::mt19937 random(2173831);
    for (int test = 0; test < 800; ++test)
    {
        const int n = static_cast<int>(random() % 9) + 1;
        const int m = static_cast<int>(random() % 31);
        std::vector<int> potentials(n);
        for (int &value : potentials)
        {
            value = static_cast<int>(random() % 31) - 15;
        }
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            const int u = static_cast<int>(random() % n);
            const int v = static_cast<int>(random() % n);
            const int weight = static_cast<int>(random() % 15) + potentials[v] - potentials[u] - (test % 2 == 0 ? 3 : 0);
            edges.push_back({u, v, weight});
        }
        check(n, edges);
    }
#ifndef SKIP_RUNTIME_CHECKS
    auto ignore = [](int, auto) {};
    check_error<std::length_error>([&] { (void)sirkit::for_each_johnson_distance_row(-1, {}, ignore); });
    constexpr auto kBad = std::numeric_limits<std::int64_t>::max() / 8 + 1;
    for (const Edge edge : std::vector<Edge>{{-1, 0, 1}, {2, 0, 1}, {0, -1, 1}, {0, 2, 1}, {0, 1, -kBad}, {0, 1, kBad}})
    {
        check_error<std::out_of_range>(
            [&] { (void)sirkit::for_each_johnson_distance_row(2, std::span(&edge, 1), ignore); });
    }
#endif
}
