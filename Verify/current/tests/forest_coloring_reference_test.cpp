// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Forest_Coloring.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

int brute_colorings(int v, std::vector<int> &colors, int k, const std::vector<std::vector<bool>> &conflicts)
{
    if (v == static_cast<int>(colors.size()))
    {
        return 1;
    }
    int total = 0;
    for (int color = 0; color < k; ++color)
    {
        bool possible = true;
        for (int earlier = 0; earlier < v; ++earlier)
        {
            if (conflicts[v][earlier] && colors[earlier] == color)
            {
                possible = false;
                break;
            }
        }
        if (possible)
        {
            colors[v] = color;
            total += brute_colorings(v + 1, colors, k, conflicts);
        }
    }
    return total;
}

void check(int n, const std::vector<Edge> &edges)
{
    const sirkit::ForestColoring forest(n, edges);
    const auto [edge_count, edge_colors] = forest.edge_coloring();
    const auto [vertex_count, vertex_colors] = forest.distance_two_coloring();
    assert(edge_colors.size() == edges.size());
    assert(vertex_colors.size() == static_cast<std::size_t>(n));

    std::vector<int> degree(n);
    std::vector<std::vector<bool>> adjacency(n, std::vector<bool>(n));
    std::vector<std::set<int>> used(n);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto [u, v] = edges[i];
        const int color = edge_colors[i];
        assert(0 <= color && color < edge_count);
        assert(used[u].insert(color).second);
        assert(used[v].insert(color).second);
        ++degree[u];
        ++degree[v];
        adjacency[u][v] = adjacency[v][u] = true;
    }
    assert(edge_count == (n ? *std::max_element(degree.begin(), degree.end()) : 0));
    assert(vertex_count == edge_count + (n != 0));

    auto conflicts = adjacency;
    for (int u = 0; u < n; ++u)
    {
        assert(0 <= vertex_colors[u] && vertex_colors[u] < vertex_count);
        for (int v = 0; v < u; ++v)
        {
            for (int w = 0; w < n; ++w)
            {
                conflicts[u][v] = conflicts[u][v] || (adjacency[u][w] && adjacency[w][v]);
            }
            if (conflicts[u][v])
            {
                assert(vertex_colors[u] != vertex_colors[v]);
            }
        }
    }
    std::vector<int> colors(n);
    for (int k = 0; k <= 5; ++k)
    {
        assert(forest.count_distance_two<int>(k) == brute_colorings(0, colors, k, conflicts));
    }
}

template <class Error, class Action>
void expect_error(Action action)
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
    check(1, {});
    check(7, {});
    check(5, {{0, 1}, {1, 2}, {2, 3}, {3, 4}});
    check(7, {{0, 4}, {4, 5}, {4, 2}, {1, 3}, {6, 1}});
    check(7, {{4, 1}, {4, 6}, {3, 4}, {2, 4}, {4, 0}, {4, 5}});
    constexpr auto palette = std::numeric_limits<std::uint64_t>::max();
    assert(sirkit::ForestColoring(1, {}).count_distance_two<std::uint64_t>(palette) == palette);

    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::ForestColoring(-1, {});
                                    });
    for (const auto &[u, v] : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array invalid{Edge{u, v}};
                                            (void)sirkit::ForestColoring(2, invalid);
                                        });
    }
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array self{Edge{0, 0}};
                                            (void)sirkit::ForestColoring(1, self);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array multiple{Edge{2, 3}, Edge{3, 2}};
                                            (void)sirkit::ForestColoring(4, multiple);
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array cycle{Edge{1, 2}, Edge{2, 3}, Edge{3, 1}};
                                            (void)sirkit::ForestColoring(4, cycle);
                                        });

    std::mt19937 random(146133203);
    for (int trial = 0; trial < 360; ++trial)
    {
        const int n = static_cast<int>(random() % 7) + 1;
        std::vector<int> permutation(n);
        for (int v = 0; v < n; ++v)
        {
            permutation[v] = v;
        }
        std::shuffle(permutation.begin(), permutation.end(), random);
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            if (random() % 5 != 0)
            {
                edges.emplace_back(permutation[v], permutation[random() % v]);
            }
        }
        std::shuffle(edges.begin(), edges.end(), random);
        check(n, edges);
    }

    constexpr int kSize = 100000;
    std::vector<Edge> deep;
    for (int v = 1; v < kSize; ++v)
    {
        deep.emplace_back(v - 1, v);
    }
    const sirkit::ForestColoring chain(kSize, deep);
    assert(chain.count_distance_two<unsigned>(3) == 6);
    const auto edge_colors = chain.edge_coloring().colors;
    const auto vertex_colors = chain.distance_two_coloring().colors;
    for (int v = 0; v < kSize - 1; ++v)
    {
        assert(edge_colors[v] == v % 2);
        assert(vertex_colors[v] == v % 3);
    }
    assert(vertex_colors.back() == (kSize - 1) % 3);
}
