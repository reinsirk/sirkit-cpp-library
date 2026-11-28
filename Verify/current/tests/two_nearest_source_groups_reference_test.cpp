// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Two_Nearest_Source_Groups.cpp"

#include <algorithm>
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
using Distance = std::optional<std::uint64_t>;
using Edge = sirkit::ShortestPathEdge;
using Source = sirkit::GroupedShortestPathSource;

void check(int n, const std::vector<Edge> &edges, const std::vector<Source> &sources, bool undirected)
{
    const auto actual = sirkit::two_nearest_source_groups(n, edges, sources, undirected);
    assert(actual.size() == static_cast<std::size_t>(n));
    std::vector all(n, std::vector<Distance>(n));
    for (int v = 0; v < n; ++v)
    {
        all[v][v] = 0;
    }
    auto relax = [&](int u, int v, std::uint64_t w)
    {
        if (!all[u][v] || w < *all[u][v])
        {
            all[u][v] = w;
        }
    };
    for (const auto &[u, v, w] : edges)
    {
        relax(u, v, w);
        if (undirected)
        {
            relax(v, u, w);
        }
    }
    for (int k = 0; k < n; ++k)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                if (all[u][k] && all[k][v] && *all[u][k] <= std::numeric_limits<std::uint64_t>::max() - *all[k][v])
                {
                    relax(u, v, *all[u][k] + *all[k][v]);
                }
            }
        }
    }

    for (int v = 0; v < n; ++v)
    {
        std::map<int, std::uint64_t> by_group;
        for (const auto &[u, group] : sources)
        {
            if (all[u][v])
            {
                auto [it, inserted] = by_group.try_emplace(group, *all[u][v]);
                if (!inserted)
                {
                    it->second = std::min(it->second, *all[u][v]);
                }
            }
        }
        std::vector<std::uint64_t> ordered;
        for (const auto &entry : by_group)
        {
            ordered.push_back(entry.second);
        }
        std::ranges::sort(ordered);
        for (std::size_t i = 0; i != 2; ++i)
        {
            const auto &match = actual[v][i];
            if (i >= ordered.size())
            {
                assert(!match);
                continue;
            }
            assert(match && match->distance == ordered[i]);
            assert(all[match->source][v] == match->distance);
            assert(by_group.at(match->group) == match->distance);
            assert(std::ranges::any_of(sources, [&](const Source &source)
                                      { return source.vertex == match->source && source.group == match->group; }));
        }
        if (actual[v][1])
        {
            assert(actual[v][0]->group != actual[v][1]->group);
        }
    }
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
    check(0, {}, {}, false);
    check(3, {{0, 1, 1}}, {}, false);
    check(3, {}, {{0, -13}, {0, -13}, {0, 10}, {1, 3}}, true);
    check(6, {{0, 1, std::numeric_limits<std::uint64_t>::max()}, {1, 2, 1}, {1, 3, 0},
              {4, 3, 5}, {4, 5, 0}, {5, 4, 0}},
          {{0, -1}, {4, 0}, {4, -1}}, false);
    std::mt19937 generator(29663);
    for (int test = 0; test < 800; ++test)
    {
        const int n = static_cast<int>(generator() % 9) + 1;
        const int m = static_cast<int>(generator() % 20);
        const int s = static_cast<int>(generator() % 12);
        std::vector<Edge> edges;
        std::vector<Source> sources;
        for (int edge = 0; edge < m; ++edge)
        {
            const int u = static_cast<int>(generator() % n);
            const int v = static_cast<int>(generator() % n);
            const std::uint64_t weight = test % 2 == 0 ? 1 : generator() % 9;
            edges.push_back({u, v, weight});
        }
        for (int index = 0; index < s; ++index)
        {
            const int v = static_cast<int>(generator() % n);
            const int group = static_cast<int>(generator() % 5) - 2;
            sources.push_back({v, group});
        }
        check(n, edges, sources, test % 4 < 2);
    }
#ifndef SKIP_RUNTIME_CHECKS
    throws<std::length_error>([] { (void)sirkit::two_nearest_source_groups(-1, {}, {}); });
    for (const int vertex : {-1, 2})
    {
        const Source source{vertex, 9};
        throws<std::out_of_range>([&]
                                  { (void)sirkit::two_nearest_source_groups(2, {}, std::span(&source, 1)); });
    }
    for (const Edge edge : std::vector<Edge>{{-1, 0, 1}, {2, 0, 2}, {0, -1, 3}, {0, 2, 4}})
    {
        throws<std::out_of_range>([&]
                                  { (void)sirkit::two_nearest_source_groups(2, std::span(&edge, 1), {}); });
    }
#endif
}
