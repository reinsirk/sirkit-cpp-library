// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Complement_Shortest_Paths.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
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

void compare(int n, std::span<const Edge> edges, int source, std::uint64_t mod)
{
    std::vector<std::vector<bool>> complement(n, std::vector<bool>(n, true));
    for (int v = 0; v < n; ++v)
    {
        complement[v][v] = false;
    }
    for (const auto &[u, v] : edges)
    {
        complement[u][v] = complement[v][u] = false;
    }
    std::vector<int> distance(n, -1);
    std::vector<std::uint64_t> exact_paths(n);
    std::queue<int> pending;
    distance[source] = 0;
    exact_paths[source] = 1;
    pending.push(source);
    while (!pending.empty())
    {
        const int v = pending.front();
        pending.pop();
        for (int u = 0; u < n; ++u)
        {
            if (complement[v][u])
            {
                if (distance[u] == -1)
                {
                    distance[u] = distance[v] + 1;
                    pending.push(u);
                }
                if (distance[u] == distance[v] + 1)
                {
                    exact_paths[u] += exact_paths[v];
                }
            }
        }
    }
    const auto result = sirkit::complement_bfs(n, edges, source, mod);
    assert(result.distance == distance);
    if (mod == 0)
    {
        assert(result.paths.empty());
    }
    for (int v = 0; v < n; ++v)
    {
        if (v == source || distance[v] < 0)
        {
            assert(result.parent[v] == -1);
        }
        else
        {
            const int u = result.parent[v];
            assert(0 <= u && u < n && complement[v][u]);
            assert(distance[u] == distance[v] - 1);
        }
        if (mod != 0)
        {
            assert(result.paths[v] == exact_paths[v] % mod);
        }
    }
}

void check_limits()
{
    const auto empty = sirkit::complement_bfs(0, {});
    assert(empty.distance.empty() && empty.parent.empty() && empty.paths.empty());
    const std::array loop{Edge{0, 0}};
    compare(1, loop, 0, 1);
    compare(1, loop, 0, 0);
    const std::array bar{Edge{0, 1}, Edge{1, 0}, Edge{1, 1}};
    compare(2, bar, 1, 0);
    compare(2, bar, 1, 2);
    must_throw<std::length_error>([] { static_cast<void>(sirkit::complement_bfs(-1, {})); });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int source : {-1, 2})
    {
        must_throw<std::out_of_range>([&] { static_cast<void>(sirkit::complement_bfs(2, {}, source)); });
    }
    for (const auto &edge : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        const std::span edges(&edge, 1);
        must_throw<std::out_of_range>([&] { static_cast<void>(sirkit::complement_bfs(2, edges)); });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(319);
    constexpr std::array<std::uint64_t, 6> kModuli{0, 1, 2, 7, 998244353, std::numeric_limits<std::uint64_t>::max()};
    for (int test = 0; test < 1100; ++test)
    {
        const int n = 1 + static_cast<int>(rng() % 35);
        std::vector<Edge> edges;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u; v < n; ++v)
            {
                if (rng() % 100 < static_cast<unsigned>(test % 105))
                {
                    edges.emplace_back(u, v);
                    if (rng() % 4 == 0)
                    {
                        edges.emplace_back(v, u);
                    }
                }
            }
        }
        for (const auto mod : kModuli)
        {
            compare(n, edges, static_cast<int>(rng() % n), mod);
        }
    }
}

void check_large_modulus()
{
    constexpr int kMiddleLayers = 70;
    constexpr int kCount = 2 * kMiddleLayers + 2;
    std::vector<int> layer{0};
    for (int i = 1; i <= kMiddleLayers; ++i)
    {
        layer.push_back(i);
        layer.push_back(i);
    }
    layer.push_back(kMiddleLayers + 1);
    std::vector<Edge> forbidden;
    for (int u = 0; u < kCount; ++u)
    {
        for (int v = u + 1; v < kCount; ++v)
        {
            if (layer[v] != layer[u] + 1)
            {
                forbidden.emplace_back(u, v);
            }
        }
    }
    constexpr auto kModulus = std::numeric_limits<std::uint64_t>::max();
    const auto result = sirkit::complement_bfs(kCount, forbidden, 0, kModulus);
    assert(result.distance.back() == kMiddleLayers + 1);
    // 中間層の各2頂点を選ぶ2^70本。2^64の余り1を使う。
    assert(result.paths.back() == 64);
}
}

int main()
{
    check_limits();
    check_random();
    check_large_modulus();
}
