// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Block_Cut_Forest.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <queue>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

template<class Exception, class Operation>
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

std::vector<int> distances(const sirkit::Graph &graph, int source)
{
    std::vector<int> result(graph.size(), -1);
    std::queue<int> pending;
    pending.push(source);
    result[source] = 0;
    while (!pending.empty())
    {
        const int v = pending.front();
        pending.pop();
        for (const int u : graph[v])
        {
            if (result[u] == -1)
            {
                result[u] = result[v] + 1;
                pending.push(u);
            }
        }
    }
    return result;
}

bool brute_path(const sirkit::Graph &graph, int v, int middle, int target, unsigned used)
{
    if (v == target)
    {
        return ((used >> middle) & 1) != 0;
    }
    for (const int u : graph[v])
    {
        if ((((used >> u) & 1) == 0) && brute_path(graph, u, middle, target, used | (1U << u)))
        {
            return true;
        }
    }
    return false;
}

void validate_forest(const sirkit::Graph &forest, int n)
{
    std::size_t arcs = 0;
    for (int v = 0; v < static_cast<int>(forest.size()); ++v)
    {
        arcs += forest[v].size();
        for (const int u : forest[v])
        {
            assert(0 <= u && static_cast<std::size_t>(u) < forest.size());
            assert((v < n) != (u < n));
            assert(std::count(forest[u].begin(), forest[u].end(), v) == 1);
        }
    }
    std::vector<bool> seen(forest.size());
    std::size_t components = 0;
    for (int v = 0; v < static_cast<int>(forest.size()); ++v)
    {
        if (!seen[v])
        {
            ++components;
            const auto distance = distances(forest, v);
            for (std::size_t u = 0; u < distance.size(); ++u)
            {
                seen[u] = seen[u] || distance[u] != -1;
            }
        }
    }
    assert(arcs / 2 + components == forest.size());
}

void check_limits()
{
    assert(sirkit::block_cut_forest(0, {}).empty());
    const std::array pairs{Edge{0, 0}, Edge{0, 0}, Edge{0, 1}, Edge{1, 0}, Edge{1, 2}};
    const auto forest = sirkit::block_cut_forest(4, pairs);
    validate_forest(forest, 4);
    assert(forest.size() == 8);
    assert(forest[0].size() == 3 && forest[1].size() == 2 && forest[2].size() == 1 && forest[3].empty());
    must_throw<std::length_error>([] { static_cast<void>(sirkit::block_cut_forest(-1, {})); });
    must_throw<std::length_error>([] {
        static_cast<void>(sirkit::detail::block_cut_forest_size(std::numeric_limits<int>::max(), 1));
    });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &edge : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        const std::span edges(&edge, 1);
        must_throw<std::out_of_range>([&] { static_cast<void>(sirkit::block_cut_forest(2, edges)); });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(318);
    for (int test = 0; test < 120; ++test)
    {
        const int n = 2 + static_cast<int>(rng() % 6);
        std::vector<Edge> edges;
        std::vector<std::vector<int>> simple(n);
        for (int u = 0; u < n; ++u)
        {
            for (int v = u; v < n; ++v)
            {
                if (rng() % 100 < static_cast<unsigned>(test % 97))
                {
                    if (u != v)
                    {
                        simple[u].push_back(v);
                        simple[v].push_back(u);
                    }
                    edges.emplace_back(u, v);
                    if (rng() % 3 == 0)
                    {
                        edges.emplace_back(v, u);
                    }
                }
            }
        }
        const auto forest = sirkit::block_cut_forest(n, edges);
        validate_forest(forest, n);
        std::vector<std::vector<int>> distances_in_forest(n);
        for (int v = 0; v < n; ++v)
        {
            distances_in_forest[v] = distances(forest, v);
        }
        for (int a = 0; a < n; ++a)
        {
            for (int c = 0; c < n; ++c)
            {
                for (int b = 0; b < n; ++b)
                {
                    const auto &from_a = distances_in_forest[a];
                    const auto &from_b = distances_in_forest[b];
                    const bool through = from_a[c] != -1 && from_b[a] != -1 &&
                                         from_b[a] + from_b[c] - from_a[c] <= 2;
                    assert(through == brute_path(simple, a, b, c, 1U << a));
                }
            }
        }
    }
}
}

int main()
{
    check_limits();
    check_random();
}
