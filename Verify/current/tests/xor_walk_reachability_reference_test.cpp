// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Xor_Walk_Reachability.cpp"

#include <array>
#include <cassert>
#include <limits>
#include <random>
#include <vector>

namespace
{
using Edge = sirkit::XorWalkEdge;

void check(int n, const std::vector<Edge> &edges, int source, bool directed)
{
    using State = std::array<bool, 16>;
    std::vector<State> discovered(n);
    discovered[source][0] = true;
    for (bool changed = true; changed;)
    {
        changed = false;
        const auto previous = discovered;
        for (const auto &[u, v, weight] : edges)
        {
            for (std::uint64_t k = 0; k < 16; ++k)
            {
                if (previous[u][k] && !discovered[v][k ^ weight])
                {
                    discovered[v][k ^ weight] = true;
                    changed = true;
                }
                if (!directed && previous[v][k] && !discovered[u][k ^ weight])
                {
                    discovered[u][k ^ weight] = true;
                    changed = true;
                }
            }
        }
    }

    const sirkit::XorWalkReachability walks(n, edges, source, directed);
    for (int vertex = 0; vertex < n; ++vertex)
    {
        std::optional<std::uint64_t> low, high;
        for (std::uint64_t value = 0; value < 16; ++value)
        {
            assert(walks.reachable(vertex, value) == discovered[vertex][value]);
            if (discovered[vertex][value])
            {
                low = low.value_or(value);
                high = value;
            }
        }
        assert(walks.min_xor(vertex) == low && walks.max_xor(vertex) == high);
        assert(!walks.reachable(vertex, std::numeric_limits<std::uint64_t>::max()));
    }
}
} // namespace

int main()
{
    check(2, {}, 1, true);
    check(4, {{0, 1, 0}, {1, 2, 1}, {1, 1, 2}, {2, 0, 4}}, 0, true);
    check(4, {{0, 1, 0}, {1, 2, 1}, {1, 1, 2}, {2, 0, 4}}, 2, false);

    std::mt19937 rng(71810);
    for (int trial = 0; trial < 1200; ++trial)
    {
        const int n = 1 + rng() % 8;
        const int m = rng() % 16;
        const int source = rng() % n;
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            edges.push_back({static_cast<int>(rng() % n), static_cast<int>(rng() % n), rng() % 16});
        }
        check(n, edges, source, true);
        check(n, edges, source, false);
    }

    constexpr auto kHigh = std::uint64_t{1} << 63;
    const auto check_large = [](std::uint64_t weight)
    {
        const Edge edge{0, 1, weight};
        try
        {
            (void)sirkit::XorWalkReachability(2, std::span(&edge, 1), 0);
        }
        catch (const std::length_error &)
        {
            return true;
        }
        return false;
    };
    assert(check_large(kHigh));
    assert(check_large(kHigh >> 1));

#ifndef SKIP_RUNTIME_CHECKS
    int rejected = 0;
    for (const int source : {-1, 2})
    {
        try
        {
            (void)sirkit::XorWalkReachability(2, {}, source);
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    const Edge bad_edge{0, 2, 0};
    try
    {
        (void)sirkit::XorWalkReachability(2, std::span(&bad_edge, 1), 0);
    }
    catch (const std::out_of_range &)
    {
        ++rejected;
    }
    const sirkit::XorWalkReachability graph(1, {}, 0);
    try
    {
        (void)graph.reachable(1, 0);
    }
    catch (const std::out_of_range &)
    {
        ++rejected;
    }
    assert(rejected == 4);
#endif
}
