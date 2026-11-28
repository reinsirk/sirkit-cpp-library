// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Parity_Orientation.cpp"

#include <array>
#include <cassert>
#include <random>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

void check(int n, const std::vector<Edge> &edges, std::mt19937 &rng)
{
    std::vector<bool> possible(1U << n);
    for (unsigned orientation = 0; orientation < (1U << edges.size()); ++orientation)
    {
        unsigned parity = 0;
        for (unsigned i = 0; i < edges.size(); ++i)
        {
            const auto [u, v] = edges[i];
            parity ^= 1U << ((orientation >> i) & 1 ? u : v);
        }
        possible[parity] = true;
    }

    for (int trial = 0; trial < 5; ++trial)
    {
        const unsigned target = trial == 0 ? 0 : rng() % possible.size();
        std::vector<std::uint8_t> required(n);
        for (int v = 0; v < n; ++v)
        {
            required[v] = (target >> v) & 1;
        }
        const auto result = trial == 0 ? sirkit::orient_with_outdegree_parity(n, edges)
                                       : sirkit::orient_with_outdegree_parity(n, edges, required);
        assert(result.has_value() == possible[target]);
        if (result)
        {
            assert(result->size() == edges.size());
            unsigned parity = 0;
            for (std::size_t i = 0; i < edges.size(); ++i)
            {
                const auto [from, to] = (*result)[i];
                assert((edges[i] == Edge{from, to} || edges[i] == Edge{to, from}));
                parity ^= 1U << from;
            }
            assert(parity == target);
        }
    }
}
} // namespace

int main()
{
    std::mt19937 rng(40318);
    check(0, {}, rng);
    check(7, {{1, 0}, {1, 2}, {2, 0}, {2, 2}, {1, 0}, {3, 4}}, rng);
    check(6, {{1, 0}, {1, 2}, {2, 0}, {0, 3}, {3, 3}, {4, 5}, {5, 4}}, rng);
    check(4, {}, rng);
    for (int trial = 0; trial < 950; ++trial)
    {
        const int n = 1 + rng() % 8;
        const int m = rng() % 13;
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            edges.emplace_back(rng() % n, rng() % n);
        }
        check(n, edges, rng);
    }

#ifndef SKIP_RUNTIME_CHECKS
    int rejected = 0;
    try
    {
        (void)sirkit::orient_with_outdegree_parity(-1, {});
    }
    catch (const std::invalid_argument &)
    {
        ++rejected;
    }
    for (const auto &parity : std::array{std::vector<std::uint8_t>{0}, std::vector<std::uint8_t>{0, 2}})
    {
        try
        {
            (void)sirkit::orient_with_outdegree_parity(2, {}, parity);
        }
        catch (const std::invalid_argument &)
        {
            ++rejected;
        }
    }
    for (const auto &edge : std::array{Edge{-1, 0}, Edge{1, 0}, Edge{0, -1}, Edge{0, 1}})
    {
        try
        {
            (void)sirkit::orient_with_outdegree_parity(1, std::span(&edge, 1));
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    assert(rejected == 7);
#endif
}
