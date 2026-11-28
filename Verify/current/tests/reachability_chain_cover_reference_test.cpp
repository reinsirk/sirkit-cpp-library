// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Reachability_Chain_Cover.cpp"

#include <cassert>
#include <limits>
#include <random>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

void check(int n, const std::vector<Edge> &arcs)
{
    std::vector reach(n, std::vector<bool>(n));
    for (const auto &[u, v] : arcs)
    {
        reach[u][v] = true;
    }
    for (int v = 0; v < n; ++v)
    {
        reach[v][v] = true;
    }
    for (int k = 0; k < n; ++k)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                reach[u][v] = reach[u][v] || (reach[u][k] && reach[k][v]);
            }
        }
    }

    const auto cover = sirkit::minimum_reachability_chain_cover(n, arcs);
    std::vector<bool> seen(n);
    for (const auto &chain : cover)
    {
        assert(!chain.empty());
        for (std::size_t i = 0; i < chain.size(); ++i)
        {
            assert(!seen[chain[i]]);
            seen[chain[i]] = true;
            if (i != 0)
            {
                assert(reach[chain[i - 1]][chain[i]]);
            }
        }
    }
    assert(std::ranges::all_of(seen, [](bool used)
                               {
                                   return used;
                               }));

    // どの2頂点も比較可能な集合だけを1チェーンとして選び、全部分集合を調べる。
    const unsigned subsets = 1U << n;
    std::vector<bool> allowed(subsets, true);
    for (unsigned mask = 1; mask < subsets; ++mask)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = u + 1; v < n; ++v)
            {
                if ((mask & (1U << u)) && (mask & (1U << v)) && !reach[u][v] && !reach[v][u])
                {
                    allowed[mask] = false;
                }
            }
        }
    }

    std::vector<int> best(subsets, n);
    best[0] = 0;
    for (unsigned mask = 1; mask < subsets; ++mask)
    {
        for (unsigned remaining = mask; remaining != 0; remaining = (remaining - 1) & mask)
        {
            if (allowed[remaining])
            {
                best[mask] = std::min(best[mask], 1 + best[mask ^ remaining]);
            }
        }
    }
    assert(cover.size() == static_cast<std::size_t>(best.back()));
}
} // namespace

int main()
{
    check(0, {});
    check(6, {});
    check(7, {{0, 2}, {1, 2}, {2, 3}, {3, 4}, {3, 5}, {5, 6}, {6, 5}});
    check(8, {{0, 1}, {1, 2}, {2, 0}, {1, 3}, {1, 3}, {2, 3}, {3, 4}, {4, 4}, {6, 7}});
    std::mt19937 rng(51233);
    for (int trial = 0; trial < 930; ++trial)
    {
        const int n = 1 + rng() % 8;
        const int m = rng() % 20;
        std::vector<Edge> arcs;
        for (int i = 0; i < m; ++i)
        {
            int u = rng() % n, v = rng() % n;
            if (trial % 2 != 0 && u > v)
            {
                std::swap(u, v);
            }
            arcs.emplace_back(u, v);
        }
        check(n, arcs);
    }

    bool size_rejected = false;
    try
    {
        sirkit::detail::check_chain_cover_network_size(std::numeric_limits<int>::max());
    }
    catch (const std::length_error &)
    {
        size_rejected = true;
    }
    assert(size_rejected);

#ifndef SKIP_RUNTIME_CHECKS
    bool vertex_rejected = false;
    const std::vector<Edge> wrong{{0, 1}};
    try
    {
        (void)sirkit::minimum_reachability_chain_cover(1, wrong);
    }
    catch (const std::out_of_range &)
    {
        vertex_rejected = true;
    }
    assert(vertex_rejected);
#endif
}
