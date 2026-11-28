// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Eulerian_Trail_Decomposition.cpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

template <bool Directed>
void check(int n, const std::vector<Edge> &edges)
{
    const auto trails = sirkit::minimum_eulerian_trail_decomposition<Directed>(n, edges);
    std::vector<int> used(edges.size());
    for (const auto &trail : trails)
    {
        assert(!trail.edges.empty());
        assert(trail.vertices.size() == trail.edges.size() + 1);
        for (std::size_t i = 0; i < trail.edges.size(); ++i)
        {
            const std::size_t id = trail.edges[i];
            assert(id < edges.size());
            ++used[id];
            const Edge traversed{trail.vertices[i], trail.vertices[i + 1]};
            const auto [u, v] = edges[id];
            assert((traversed == edges[id] || (!Directed && traversed == Edge{v, u})));
        }
    }
    for (const int count : used)
    {
        assert(count == 1);
    }

    // Enumerate trail edge sets without using a degree or connectivity criterion.
    const int mask_count = 1 << edges.size();
    std::vector<bool> is_trail(mask_count);
    std::vector seen(mask_count, std::vector<bool>(n));
    auto explore = [&](auto &self, int u, int mask) -> void
    {
        if (seen[mask][u])
        {
            return;
        }
        seen[mask][u] = true;
        is_trail[mask] = true;
        for (std::size_t id = 0; id < edges.size(); ++id)
        {
            const int bit = 1 << id;
            if (mask & bit)
            {
                continue;
            }
            const auto [a, b] = edges[id];
            if (u == a)
            {
                self(self, b, mask | bit);
            }
            if (!Directed && u == b)
            {
                self(self, a, mask | bit);
            }
        }
    };
    for (int u = 0; u < n; ++u)
    {
        explore(explore, u, 0);
    }
    std::vector<int> best(mask_count, static_cast<int>(edges.size()));
    best[0] = 0;
    for (int mask = 1; mask < mask_count; ++mask)
    {
        for (int sub = mask; sub != 0; sub = (sub - 1) & mask)
        {
            if (is_trail[sub])
            {
                best[mask] = std::min(best[mask], 1 + best[mask ^ sub]);
            }
        }
    }
    assert(trails.size() == static_cast<std::size_t>(best.back()));
}

template <class Error, class Function>
void throws(Function function)
{
    bool caught = false;
    try
    {
        function();
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
    check<false>(0, {});
    check<true>(0, {});
    check<false>(5, {{0, 1}, {0, 2}, {0, 3}, {0, 3}, {3, 3}, {4, 4}});
    check<true>(6, {{0, 1}, {0, 1}, {0, 2}, {2, 3}, {3, 3}, {4, 4}, {4, 4}});
    check<true>(6, {{1, 2}, {2, 1}, {2, 3}, {3, 2}, {4, 4}});
    std::mt19937 random(17829);
    for (int test = 0; test < 850; ++test)
    {
        const int n = static_cast<int>(random() % 6) + 1;
        const int m = static_cast<int>(random() % 9);
        std::vector<Edge> edges;
        for (int j = 0; j < m; ++j)
        {
            const int u = static_cast<int>(random() % n);
            const int v = static_cast<int>(random() % n);
            edges.emplace_back(u, v);
        }
        check<true>(n, edges);
        check<false>(n, edges);
    }
#ifndef SKIP_RUNTIME_CHECKS
    throws<std::length_error>([] { (void)sirkit::minimum_eulerian_trail_decomposition(-1, {}); });
    throws<std::length_error>(
        [] { (void)sirkit::minimum_eulerian_trail_decomposition(std::numeric_limits<int>::max(), {}); });
    for (const Edge edge : std::vector<Edge>{{-1, 0}, {2, 0}, {0, -1}, {0, 2}})
    {
        throws<std::out_of_range>(
            [&] { (void)sirkit::minimum_eulerian_trail_decomposition(2, std::span(&edge, 1)); });
        throws<std::out_of_range>(
            [&] { (void)sirkit::minimum_eulerian_trail_decomposition<true>(2, std::span(&edge, 1)); });
    }
#endif
}
