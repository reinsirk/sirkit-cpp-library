// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Graph/Complement_Shortest_Paths.cpp
// 参考（距離ごとの経路数の集計）: https://atcoder.jp/contests/abc319/editorial/7121
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/complement_shortest_paths.md
//
// API一覧
// complement_bfs(n,edges,source=0,modulus=0): 無向補グラフで幅優先探索。頂点番号は[0,n)。
// edgesは元の無向辺。自己辺・平行辺は結果に影響しない。n=0ならsource=0として空の配列を返す。
// ComplementBfsResult{distance,parent,paths}: 辺数距離、1つの最短経路での直前の頂点、最短経路数。
// 到達不能の距離・直前頂点は-1。始点の直前頂点も-1。経路数はmodulusでの剰余、到達不能なら0。
// modulus=0なら経路数を数えずpathsは空。補辺を作らずO(n+m)時間・領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COMPLEMENT_SHORTEST_PATHS_DEFINED
#define SIRKIT_COMPLEMENT_SHORTEST_PATHS_DEFINED

struct ComplementBfsResult
{
    std::vector<int> distance;
    std::vector<int> parent;
    std::vector<std::uint64_t> paths;
};

inline ComplementBfsResult complement_bfs(int n, std::span<const std::pair<int, int>> edges,
                                          int source = 0, std::uint64_t modulus = 0)
{
    if (n < 0)
    {
        throw std::length_error("complement_bfs: negative vertex count");
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= std::max(n, 1))
    {
        throw std::out_of_range("complement_bfs: source vertex");
    }
#endif
    std::vector<std::vector<int>> graph(n);
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("complement_bfs: edge endpoint");
        }
#endif
        graph[u].push_back(v);
        graph[v].push_back(u);
    }
    if (n == 0)
    {
        return {};
    }
    ComplementBfsResult result{std::vector<int>(n, -1), std::vector<int>(n, -1), {}};
    std::vector<int> next(n), neighbor_of(n, -1), visited;
    visited.reserve(n);
    std::iota(next.begin(), next.end(), std::uint32_t{1});
    int head = 0;
    int &before_source = source == 0 ? head : next[source - 1];
    before_source = source + 1;
    visited.push_back(source);
    result.distance[source] = 0;
    for (std::size_t cursor = 0; cursor < visited.size(); ++cursor)
    {
        const int u = visited[cursor];
        for (const int v : graph[u])
        {
            neighbor_of[v] = u;
        }
        // 未訪問のまま残す候補は元グラフの隣接頂点に限られる。
        int *link = &head;
        while (*link != n)
        {
            const int v = *link;
            if (neighbor_of[v] == u)
            {
                link = &next[v];
            }
            else
            {
                *link = next[v];
                result.distance[v] = result.distance[u] + 1;
                result.parent[v] = u;
                visited.push_back(v);
            }
        }
    }
    if (modulus == 0)
    {
        return result;
    }
    result.paths.resize(n);
    std::vector<std::uint64_t> layer_total(static_cast<std::size_t>(result.distance[visited.back()]) + 1);
    layer_total[0] = result.paths[source] = 1 % modulus;
    std::fill(neighbor_of.begin(), neighbor_of.end(), -1);
    for (std::size_t cursor = 1; cursor < visited.size(); ++cursor)
    {
        const int u = visited[cursor];
        const int layer = result.distance[u];
        auto count = layer_total[layer - 1];
        for (const int v : graph[u])
        {
            if (result.distance[v] == layer - 1 && neighbor_of[v] != u)
            {
                neighbor_of[v] = u;
                // 直前の層のうち、補グラフで辺のない頂点の経路数を引く。
                const auto invalid = result.paths[v];
                count = count >= invalid ? count - invalid : modulus - (invalid - count);
            }
        }
        result.paths[u] = count;
        auto &total = layer_total[layer];
        total = total >= modulus - count ? total - (modulus - count) : total + count;
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
