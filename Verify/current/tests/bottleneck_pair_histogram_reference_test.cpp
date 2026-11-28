// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Bottleneck_Pair_Histogram.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Edge = sirkit::BottleneckEdge;

void check(int n, const std::vector<Edge> &edges)
{
    using Weight = std::optional<std::int64_t>;
    std::vector minimax(n, std::vector<Weight>(n));
    std::vector maximin(n, std::vector<Weight>(n));
    for (int v = 0; v < n; ++v)
    {
        minimax[v][v] = std::numeric_limits<std::int64_t>::min();
        maximin[v][v] = std::numeric_limits<std::int64_t>::max();
    }
    auto relax_minimax = [&](int u, int v, std::int64_t w)
    {
        if (!minimax[u][v] || w < *minimax[u][v])
        {
            minimax[u][v] = w;
        }
    };
    auto relax_maximin = [&](int u, int v, std::int64_t w)
    {
        if (!maximin[u][v] || w > *maximin[u][v])
        {
            maximin[u][v] = w;
        }
    };
    std::map<std::int64_t, std::array<std::uint64_t, 2>> expected;
    for (const auto &[u, v, w] : edges)
    {
        relax_minimax(u, v, w);
        relax_minimax(v, u, w);
        relax_maximin(u, v, w);
        relax_maximin(v, u, w);
        expected.try_emplace(w);
    }
    for (int k = 0; k < n; ++k)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                if (minimax[u][k] && minimax[k][v])
                {
                    relax_minimax(u, v, std::max(*minimax[u][k], *minimax[k][v]));
                }
                if (maximin[u][k] && maximin[k][v])
                {
                    relax_maximin(u, v, std::min(*maximin[u][k], *maximin[k][v]));
                }
            }
        }
    }
    for (int u = 0; u < n; ++u)
    {
        for (int v = u + 1; v < n; ++v)
        {
            assert(minimax[u][v].has_value() == maximin[u][v].has_value());
            if (minimax[u][v])
            {
                ++expected.at(*minimax[u][v])[0];
                ++expected.at(*maximin[u][v])[1];
            }
        }
    }
    const auto actual = sirkit::bottleneck_pair_histogram(n, edges);
    assert(actual.size() == expected.size());
    auto it = expected.begin();
    for (const auto &[w, low, high] : actual)
    {
        assert(w == it->first);
        assert(low == it->second[0]);
        assert(high == it->second[1]);
        ++it;
    }
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
    check(8, {});
    check(5, {{0, 1, -3}, {0, 1, 2}, {1, 2, 4}, {2, 0, 5}, {4, 4, 9}, {1, 1, -6}});
    check(4, {{0, 1, std::numeric_limits<std::int64_t>::min()},
              {1, 2, 0},
              {2, 3, std::numeric_limits<std::int64_t>::max()},
              {0, 3, std::numeric_limits<std::int64_t>::max()}});
    std::mt19937 random(509101);
    for (int test = 0; test < 710; ++test)
    {
        const int n = static_cast<int>(random() % 10) + 1;
        const int m = static_cast<int>(random() % 29);
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            const int u = static_cast<int>(random() % n);
            const int v = static_cast<int>(random() % n);
            edges.push_back({u, v, static_cast<int>(random() % 17) - 8});
        }
        check(n, edges);
    }
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::length_error>([] { (void)sirkit::bottleneck_pair_histogram(
                                             static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1, {}); });
    for (const Edge edge : std::vector<Edge>{{-1, 0, 1}, {2, 0, 1}, {0, -1, 1}, {0, 2, 1}})
    {
        check_error<std::out_of_range>(
            [&] { (void)sirkit::bottleneck_pair_histogram(2, std::span(&edge, 1)); });
    }
#endif
}
