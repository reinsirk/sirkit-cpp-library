// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Dynamic_Tree_Distances.cpp"

#include <array>
#include <cassert>
#include <limits>
#include <random>
#include <vector>

namespace
{
using Edge = sirkit::DynamicTreeDistanceEdge<>;

std::optional<std::int64_t> brute(int n, const std::vector<Edge> &edges, int start, int target)
{
    std::vector<std::optional<std::int64_t>> result(n);
    std::vector<int> pending{start};
    result[start] = 0;
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const int u = pending[i];
        for (const auto &[a, b, weight] : edges)
        {
            if (u != a && u != b)
            {
                continue;
            }
            const int v = u == a ? b : a;
            if (!result[v])
            {
                result[v] = *result[u] + weight;
                pending.push_back(v);
            }
        }
    }
    return result[target];
}
} // namespace

int main()
{
    const sirkit::DynamicTreeDistances<> empty(0, {});
    assert(empty.size() == 0 && empty.edge_count() == 0);
    sirkit::DynamicTreeDistances<> one(1, {});
    assert(one.distance(0, 0) == 0 && one.root_distance(0) == 0);

    std::mt19937 rng(125805);
    for (int trial = 0; trial < 750; ++trial)
    {
        const int n = 1 + rng() % 28;
        const int root = rng() % n;
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            if (rng() % 5 != 0)
            {
                const auto p = static_cast<int>(rng() % v);
                const int weight = static_cast<int>(rng() % 2001) - 1000;
                if (rng() % 2 == 0)
                {
                    edges.push_back({p, v, weight});
                }
                else
                {
                    edges.push_back({v, p, weight});
                }
            }
        }
        sirkit::DynamicTreeDistances<> paths(n, edges, root);
        assert(paths.size() == static_cast<std::size_t>(n) && paths.edge_count() == edges.size());
        for (int i = 0; i < 70; ++i)
        {
            if (!edges.empty() && rng() % 3 == 0)
            {
                const auto id = rng() % edges.size();
                const auto w = static_cast<int>(rng() % 2001) - 1000;
                paths.set_weight(id, w);
                edges[id].weight = w;
                assert(paths.weight(id) == w);
            }
            const int u = rng() % n, v = rng() % n;
            auto expected = brute(n, edges, root, u);
            for (int r = 0; !expected && r < n; ++r)
            {
                expected = brute(n, edges, r, u);
            }
            assert(expected && paths.root_distance(u) == expected);
            assert(paths.distance(u, v) == brute(n, edges, u, v));
        }
    }

    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    const std::array high{Edge{0, 1, kMax}, Edge{1, 2, 1}, Edge{2, 3, -1}};
    sirkit::DynamicTreeDistances<> paths(4, high);
    assert(paths.root_distance(2) == kMin && paths.distance(2, 3) == -1);
    assert(paths.distance(0, 3) == kMax);
    auto copy = paths;
    copy.set_weight(0, kMin);
    assert(copy.distance(0, 3) == kMin && paths.distance(0, 3) == kMax);

    using NarrowEdge = sirkit::DynamicTreeDistanceEdge<std::int16_t>;
    const std::array narrow{NarrowEdge{0, 1, 32767}, NarrowEdge{1, 2, 2}};
    sirkit::DynamicTreeDistances<std::int16_t> small(3, narrow);
    assert(small.distance(0, 2) == -32767);
    small.set_weight(0, -32768);
    assert(small.distance(0, 2) == -32766);

    // 同じ翻訳単位でも公開のFenwick APIをそのまま使える。
    sirkit::fenwick_tree<std::uint64_t> fenwick(2);
    fenwick.set(1, 4);
    assert(fenwick.get(1) == 4 && fenwick.sum(0, 2) == 4);

#ifndef SKIP_RUNTIME_CHECKS
    int rejected = 0;
    try
    {
        (void)paths.weight(3);
    }
    catch (const std::out_of_range &)
    {
        ++rejected;
    }
    try
    {
        paths.set_weight(3, 0);
    }
    catch (const std::out_of_range &)
    {
        ++rejected;
    }
    try
    {
        (void)paths.root_distance(-1);
    }
    catch (const std::out_of_range &)
    {
        ++rejected;
    }
    try
    {
        (void)paths.distance(0, 4);
    }
    catch (const std::out_of_range &)
    {
        ++rejected;
    }
    assert(rejected == 4);
#endif
}
