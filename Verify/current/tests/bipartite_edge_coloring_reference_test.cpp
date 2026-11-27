// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Bipartite_Edge_Coloring.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

void check(int left, int right, const std::vector<Edge> &edges)
{
    const auto actual = sirkit::bipartite_edge_coloring(left, right, edges);
    assert(actual.colors.size() == edges.size());
    std::vector<std::size_t> degrees(left + right);
    std::size_t maximum = 0;
    std::vector<std::pair<int, std::size_t>> at_vertex;
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &[u, v] = edges[i];
        maximum = std::max(maximum, ++degrees[u]);
        maximum = std::max(maximum, ++degrees[left + v]);
        const auto color = actual.colors[i];
        assert(color < actual.color_count);
        at_vertex.emplace_back(u, color);
        at_vertex.emplace_back(left + v, color);
    }
    assert(actual.color_count == maximum);
    std::sort(at_vertex.begin(), at_vertex.end());
    assert(std::adjacent_find(at_vertex.begin(), at_vertex.end()) == at_vertex.end());
}

template <class Exception>
void expect_error(int l, int r, std::initializer_list<Edge> edges)
{
    try
    {
        static_cast<void>(sirkit::bipartite_edge_coloring(l, r, {edges.begin(), edges.size()}));
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
} // namespace

int main()
{
    check(0, 0, {});
    check(0, 45, {});
    check(5, 6, {{1, 2}, {2, 3}, {0, 1}});
    check(2, 3, {{0, 1}, {0, 1}, {0, 0}, {0, 2}});
    check(4, 5, {{0, 0}, {0, 0}, {0, 1}, {1, 0}, {1, 1}, {2, 4}, {2, 3}});

    constexpr int kGraphCount = 19683; // 3^9
    for (int mask = 0; mask < kGraphCount; ++mask)
    {
        std::vector<Edge> edges;
        int code = mask;
        for (int u = 0; u < 3; ++u)
        {
            for (int v = 0; v < 3; ++v)
            {
                for (int copies = code % 3; copies > 0; --copies)
                {
                    edges.emplace_back(u, v);
                }
                code /= 3;
            }
        }
        check(3, 3, edges);
    }

    std::mt19937 random(92134);
    for (int trial = 0; trial < 2000; ++trial)
    {
        const int left = 1 + static_cast<int>(random() % 40);
        const int right = 1 + static_cast<int>(random() % 40);
        const auto m = random() % 360;
        std::vector<Edge> edges;
        for (std::size_t i = 0; i < m; ++i)
        {
            edges.emplace_back(static_cast<int>(random() % left), static_cast<int>(random() % right));
        }
        check(left, right, edges);
    }
    for (int degree = 2; degree <= 35; ++degree)
    {
        constexpr int kSize = 11;
        std::vector<Edge> edges;
        for (int u = 0; u < kSize; ++u)
        {
            for (int step = 0; step < degree; ++step)
            {
                edges.emplace_back(u, (u + step) % kSize);
            }
        }
        std::shuffle(edges.begin(), edges.end(), random);
        check(kSize, kSize, edges);
    }

    expect_error<std::length_error>(-1, 3, {});
    expect_error<std::length_error>(1, -2, {});
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::out_of_range>(2, 3, {{-1, 0}});
    expect_error<std::out_of_range>(2, 3, {{2, 0}});
    expect_error<std::out_of_range>(2, 3, {{0, -1}});
    expect_error<std::out_of_range>(2, 3, {{0, 3}});
#endif

    constexpr int kSparseDegree = 17000;
    std::vector<Edge> two_stars;
    for (int i = 0; i < kSparseDegree; ++i)
    {
        two_stars.emplace_back(0, i);
        two_stars.emplace_back(i + 1, kSparseDegree);
    }
    check(100000, 100000, two_stars);

    constexpr int kCycleSize = 15000;
    std::vector<Edge> cycle;
    for (int u = 0; u < kCycleSize; ++u)
    {
        cycle.emplace_back(u, u);
        cycle.emplace_back(u, (u + 1) % kCycleSize);
    }
    check(kCycleSize, kCycleSize, cycle);
}
