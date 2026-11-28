// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Edge_Removal_Distances.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

int without_edge(int n, const std::vector<Edge> &edges, int s, int t, bool directed, std::size_t omitted)
{
    constexpr int kUnreachable = 1000000;
    std::vector<std::vector<int>> distance(n, std::vector<int>(n, kUnreachable));
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (i != omitted)
        {
            const auto [u, v] = edges[i];
            distance[u][v] = 1;
            if (!directed)
            {
                distance[v][u] = 1;
            }
        }
    }
    for (int i = 0; i < n; ++i)
    {
        distance[i][i] = 0;
    }
    for (int middle = 0; middle < n; ++middle)
    {
        for (int u = 0; u < n; ++u)
        {
            for (int v = 0; v < n; ++v)
            {
                distance[u][v] = std::min(distance[u][v], distance[u][middle] + distance[middle][v]);
            }
        }
    }
    return distance[s][t] == kUnreachable ? -1 : distance[s][t];
}

void check(int n, const std::vector<Edge> &edges, int s, int t, bool directed)
{
    const auto actual = sirkit::edge_removal_distances(n, edges, s, t, directed);
    assert(actual.size() == edges.size());
    for (std::size_t i = 0; i < actual.size(); ++i)
    {
        assert(actual[i] == without_edge(n, edges, s, t, directed, i));
    }
}

template <class Exception, class Operation>
void must_throw(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

void check_boundaries()
{
    check(3, {}, 0, 1, true);
    check(1, {{0, 0}}, 0, 0, false);
    check(4, {{0, 1}, {1, 2}, {2, 3}}, 0, 3, true);
    check(4, {{0, 1}, {1, 2}, {2, 3}, {0, 3}}, 0, 3, false);
    check(4, {{0, 1}, {1, 2}, {0, 3}, {3, 2}}, 0, 2, true);
    check(3, {{0, 1}, {1, 0}, {1, 2}}, 0, 2, false);
    check(4, {{0, 1}, {2, 3}}, 0, 3, false);
    must_throw<std::length_error>([]
                                  {
                                      static_cast<void>(sirkit::edge_removal_distances(-1, {}, 0, 0));
                                  });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[s, t] : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(sirkit::edge_removal_distances(2, {}, s, t));
                                      });
    }
    for (const auto &bad : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(sirkit::edge_removal_distances(2, {&bad, 1}, 0, 1));
                                      });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(218);
    for (int trial = 0; trial < 850; ++trial)
    {
        const int n = 1 + static_cast<int>(rng() % 9);
        const int m = static_cast<int>(rng() % 26);
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            edges.emplace_back(rng() % n, rng() % n);
        }
        const int source = static_cast<int>(rng() % n);
        const int target = static_cast<int>(rng() % n);
        check(n, edges, source, target, true);
        check(n, edges, source, target, false);
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        static_cast<void>(sirkit::edge_removal_distances(-1, {}, 0, 0));
        return 1;
    }
    check_boundaries();
    check_random();
}
