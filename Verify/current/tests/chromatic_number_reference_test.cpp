// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Chromatic_Number.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

// 色名の付け替えを省いた全探索。包除や部分集合DPは使わない。
int backtrack_coloring(const std::vector<std::vector<int>> &graph, const std::vector<int> &order,
                       std::vector<int> &assigned, int index, int used, int upper)
{
    if (index == static_cast<int>(order.size()))
    {
        return used;
    }
    const int vertex = order[index];
    unsigned unavailable = 0;
    for (const int neighbor : graph[vertex])
    {
        if (assigned[neighbor] >= 0)
        {
            unavailable |= 1U << assigned[neighbor];
        }
    }
    for (int color = 0; color <= used && color + 1 < upper; ++color)
    {
        if ((unavailable >> color) & 1)
        {
            continue;
        }
        assigned[vertex] = color;
        upper = backtrack_coloring(graph, order, assigned, index + 1, std::max(used, color + 1), upper);
    }
    assigned[vertex] = -1;
    return upper;
}

int brute(int n, const std::vector<Edge> &edges)
{
    std::vector<std::vector<int>> graph(n);
    for (const auto &[u, v] : edges)
    {
        graph[u].push_back(v);
        graph[v].push_back(u);
    }
    std::vector<int> order(n), assigned(n, -1);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(),
              [&](int u, int v)
              {
                  return graph[u].size() > graph[v].size();
              });
    return backtrack_coloring(graph, order, assigned, 0, 0, n + 1);
}

void check(int n, const std::vector<Edge> &edges, int expected)
{
    assert(sirkit::chromatic_number(n, edges) == expected);
    const auto result = sirkit::minimum_vertex_coloring(n, edges);
    assert(result && result->color_count == expected);
    assert(result->colors.size() == static_cast<std::size_t>(n));
    std::vector<bool> used(expected);
    for (int color : result->colors)
    {
        assert(0 <= color && color < expected);
        used[color] = true;
    }
    assert(std::all_of(used.begin(), used.end(),
                       [](bool flag)
                       {
                           return flag;
                       }));
    for (const auto &[u, v] : edges)
    {
        assert(result->colors[u] != result->colors[v]);
    }
}

void check_by_backtracking(int n, const std::vector<Edge> &edges)
{
    check(n, edges, brute(n, edges));
}

std::vector<Edge> complete(int n)
{
    std::vector<Edge> edges;
    for (int u = 0; u < n; ++u)
    {
        for (int v = u + 1; v < n; ++v)
        {
            edges.emplace_back(u, v);
        }
    }
    return edges;
}
} // namespace

int main()
{
    using sirkit::chromatic_number;
    using sirkit::minimum_vertex_coloring;

    check(0, {}, 0);
    check(1, {}, 1);
    check(3, {{0, 1}, {1, 0}, {0, 1}, {1, 2}}, 2);
    const std::vector<Edge> self_loop{{1, 2}, {2, 2}};
    assert(!chromatic_number(3, self_loop));
    assert(!minimum_vertex_coloring(3, self_loop));

    for (int n = 2; n <= 5; ++n)
    {
        const auto possible = complete(n);
        for (unsigned mask = 0; mask < (1U << possible.size()); ++mask)
        {
            std::vector<Edge> edges;
            for (auto remain = mask; remain != 0; remain &= remain - 1)
            {
                edges.push_back(possible[std::countr_zero(remain)]);
            }
            check_by_backtracking(n, edges);
        }
    }
    std::mt19937 random(197701);
    for (int trial = 0; trial < 330; ++trial)
    {
        const int n = 6 + static_cast<int>(random() % 4);
        const unsigned threshold = random() % 11;
        std::vector<Edge> edges;
        for (auto edge : complete(n))
        {
            if (random() % 11 < threshold)
            {
                edges.push_back(edge);
            }
        }
        std::shuffle(edges.begin(), edges.end(), random);
        check_by_backtracking(n, edges);
    }

    // 最大頂点数。孤立点を足したクリークでは、64ビットを超える項の相殺と被覆数の両方を検査。
    check(16, {}, 1); // 桁の上限がちょうど64ビットになる判定。
    check(20, {}, 1);
    check(20, complete(20), 20);
    check(20, complete(12), 12);

    std::vector<Edge> crown;
    for (int u = 0; u < 10; ++u)
    {
        for (int v = 0; v < 10; ++v)
        {
            if (u != v)
            {
                crown.emplace_back(2 * u, 2 * v + 1);
            }
        }
    }
    check(20, crown, 2);

    std::vector<Edge> mycielski;
    for (int v = 0; v < 5; ++v)
    {
        const int next = (v + 1) % 5;
        mycielski.emplace_back(v, next);
        mycielski.emplace_back(v + 5, next);
        mycielski.emplace_back(next + 5, v);
        mycielski.emplace_back(v + 5, 10);
    }
    for (const auto &[u, v] : complete(9))
    {
        mycielski.emplace_back(u + 11, v + 11);
    }
    for (int u = 0; u < 11; ++u)
    {
        for (int v = 11; v < 20; ++v)
        {
            mycielski.emplace_back(u, v);
        }
    }
    check(20, mycielski, 13);

    for (int n : {-1, 21})
    {
        try
        {
            static_cast<void>(chromatic_number(n, {}));
            assert(false);
        }
        catch (const std::length_error &)
        {
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const Edge &invalid : std::array{Edge{-1, 0}, Edge{1, 0}, Edge{0, -1}, Edge{0, 1}})
    {
        try
        {
            static_cast<void>(chromatic_number(1, {&invalid, 1}));
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
#endif
}
