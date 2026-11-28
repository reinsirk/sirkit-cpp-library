// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Edge_Removal_Distances.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/edge_removal_distances.md
//
// API一覧
// edge_removal_distances(n, edges, source, target, directed=true):
//   辺を1本ずつ除外した場合の最短距離を、入力の辺順で返す。到達不能は-1。
//   非重み付きグラフ、頂点0..n-1。無向ではdirected=false。自己辺・多重辺可。
//   再探索する辺は最短路すべてが通る辺だけ。件数bに対しO((b+1)(n+m))時間、O(n+m)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EDGE_REMOVAL_DISTANCES_DEFINED
#define SIRKIT_EDGE_REMOVAL_DISTANCES_DEFINED

namespace detail
{
using EdgeRemovalAdjacency = std::vector<std::vector<std::pair<int, std::size_t>>>;

inline void edge_removal_bfs(const EdgeRemovalAdjacency &graph, int source, int stop,
                             std::size_t omitted, std::vector<int> &distance, std::vector<int> &pending)
{
    std::ranges::fill(distance, -1);
    pending.clear();
    pending.push_back(source);
    distance[source] = 0;
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const int u = pending[i];
        if (u == stop)
        {
            break;
        }
        for (const auto &[v, id] : graph[u])
        {
            if (id != omitted && distance[v] == -1)
            {
                distance[v] = distance[u] + 1;
                pending.push_back(v);
            }
        }
    }
}
} // namespace detail

inline std::vector<int> edge_removal_distances(int n, std::span<const std::pair<int, int>> edges,
                                               int source, int target, bool directed = true)
{
    if (n < 0)
    {
        throw std::length_error("edge_removal_distances: negative size");
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n || target < 0 || target >= n)
    {
        throw std::out_of_range("edge_removal_distances: invalid terminal");
    }
#endif
    detail::EdgeRemovalAdjacency graph(n), reverse(directed ? n : 0);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("edge_removal_distances: invalid endpoint");
        }
#endif
        graph[u].emplace_back(v, id);
        if (directed)
        {
            reverse[v].emplace_back(u, id);
        }
        else
        {
            graph[v].emplace_back(u, id);
        }
    }

    std::vector<int> pending, from_source(n);
    pending.reserve(n);
    detail::edge_removal_bfs(graph, source, -1, edges.size(), from_source, pending);
    const int original = from_source[target];
    std::vector<int> answer(edges.size(), original);
    if (original <= 0)
    {
        return answer;
    }

    std::vector<int> to_target(n);
    detail::edge_removal_bfs(directed ? reverse : graph, target, -1, edges.size(), to_target, pending);
    std::vector<std::size_t> count(original), last(original);
    auto check_layer = [&](int u, int v, std::size_t id)
    {
        const int step = from_source[u];
        if (0 <= step && step < original && to_target[v] == original - 1 - step)
        {
            ++count[step];
            last[step] = id;
        }
    };
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v] = edges[id];
        check_layer(u, v, id);
        if (!directed)
        {
            check_layer(v, u, id);
        }
    }

    std::vector<int> distance(n);
    for (int step = 0; step < original; ++step)
    {
        // Each shortest path crosses this distance layer through one edge.
        if (count[step] == 1)
        {
            const auto removed = last[step];
            detail::edge_removal_bfs(graph, source, target, removed, distance, pending);
            answer[removed] = distance[target];
        }
    }
    return answer;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
