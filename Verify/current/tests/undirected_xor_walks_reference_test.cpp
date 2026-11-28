// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Undirected_Xor_Walks.cpp"

#include <array>
#include <cassert>
#include <limits>
#include <random>
#include <vector>

namespace
{
using Edge = sirkit::XorWalkEdge;
constexpr std::uint64_t kHigh = std::uint64_t{1} << 63;

std::vector<std::array<bool, 16>> enumerate_walks(int n, std::span<const Edge> edges, int source)
{
    std::vector<std::array<bool, 16>> known(n);
    known[source][0] = true;
    for (bool changed = true; changed;)
    {
        changed = false;
        for (const auto &[u, v, weight] : edges)
        {
            for (std::uint64_t x = 0; x < 16; ++x)
            {
                if (known[u][x] && !known[v][x ^ weight])
                {
                    known[v][x ^ weight] = true;
                    changed = true;
                }
                if (known[v][x] && !known[u][x ^ weight])
                {
                    known[u][x ^ weight] = true;
                    changed = true;
                }
            }
        }
    }
    return known;
}

void check(int n, const std::vector<Edge> &edges)
{
    const sirkit::UndirectedXorWalks walks(n, edges);
    std::array<std::uint64_t, 16> counts{};
    for (int u = 0; u < n; ++u)
    {
        const auto expected = enumerate_walks(n, edges, u);
        for (int v = 0; v < n; ++v)
        {
            std::optional<std::uint64_t> lowest, highest;
            for (std::uint64_t x = 0; x < 16; ++x)
            {
                assert(walks.attainable(u, v, x) == expected[v][x]);
                if (expected[v][x])
                {
                    lowest = lowest.value_or(x);
                    highest = x;
                }
            }
            assert(walks.min_xor(u, v) == lowest);
            assert(walks.max_xor(u, v) == highest);
            assert(!walks.attainable(u, v, 16));
            if (u < v && lowest)
            {
                for (auto k = *lowest; k < 16; ++k)
                {
                    ++counts[k];
                }
            }
        }
    }
    for (std::uint64_t k = 0; k < 16; ++k)
    {
        assert(walks.count_pairs_at_most(k) == counts[k]);
    }
    assert(walks.count_pairs_at_most(std::numeric_limits<std::uint64_t>::max()) == counts.back());
}
} // namespace

int main()
{
    check(0, {});
    check(4, {});
    check(5, {{1, 0, 5}, {2, 1, 7}, {1, 2, 3}, {3, 3, 14}});
    check(5, {{0, 1, 1}, {2, 3, 4}, {1, 1, 1}, {4, 4, 8}});
    std::mt19937 rng(20261128);
    for (int trial = 0; trial < 1000; ++trial)
    {
        const int n = 1 + rng() % 8;
        const int m = rng() % 18;
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            edges.push_back({static_cast<int>(rng() % n), static_cast<int>(rng() % n), rng() % 16});
        }
        check(n, edges);
    }

    const std::array<Edge, 4> high{{{0, 1, kHigh}, {1, 2, 5}, {3, 4, kHigh | 9}, {4, 4, kHigh}}};
    const sirkit::UndirectedXorWalks walks(5, high);
    assert(walks.min_xor(0, 2) == (kHigh | 5));
    assert(walks.min_xor(3, 4) == 9 && walks.max_xor(3, 4) == (kHigh | 9));
    assert(walks.attainable(3, 4, kHigh | 9) && !walks.attainable(0, 4, 0));
    assert(walks.count_pairs_at_most(kHigh - 1) == 2);
    assert(walks.count_pairs_at_most(kHigh) == 3);
    assert(walks.count_pairs_at_most(std::numeric_limits<std::uint64_t>::max()) == 4);

    sirkit::XorBasis basis;
    assert(basis.insert(kHigh));
    assert(basis.kth(1) == kHigh);
    const sirkit::PairXorOrder pair_order(std::vector<std::uint64_t>{1, 2, 5});
    assert(pair_order.kth(1) == 4);

#ifndef SKIP_RUNTIME_CHECKS
    bool bad_size = false;
    try
    {
        (void)sirkit::UndirectedXorWalks(-1, {});
    }
    catch (const std::invalid_argument &)
    {
        bad_size = true;
    }
    assert(bad_size);
    int rejected = 0;
    for (const auto &input : std::array{Edge{-1, 0, 0}, Edge{0, 1, 0}})
    {
        try
        {
            (void)sirkit::UndirectedXorWalks(1, std::span(&input, 1));
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    for (const auto &query : std::array{Edge{-1, 0, 0}, Edge{0, 5, 0}})
    {
        try
        {
            (void)walks.min_xor(query.from, query.to);
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    assert(rejected == 4);
#endif
}
