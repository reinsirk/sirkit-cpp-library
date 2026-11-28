// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Shortest_Walk_Counts.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_walk_counts.md
//
// API一覧
// ShortestPathEdge{from,to,weight}: 0始まりの両端点と非負のuint64_t重み。
// count_shortest_walks<Count=uint64_t>(n,edges,source,undirected=false):
//   始点から各頂点への最短歩道を、辺の列が異なれば別々に数える。自己ループ・多重辺可。
//   結果のdistance[v]は最短距離。到達不能またはuint64_tで表せない距離はnullopt。
//   count[v]は有限の場合の個数、infinite[v]は最短歩道が無限にあるか。未到達・無限のcountは0。
//   Countは0,1から構築でき、加算代入を持つ型。既定のuint64_tは2^64で周回。
//   重みが全部1ならO(n+m)時間、それ以外はO((n+m)log(n+m+1))時間。O(n+m)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SHORTEST_PATH_DISTANCE_DEFINED
#define SIRKIT_SHORTEST_PATH_DISTANCE_DEFINED

struct ShortestPathEdge
{
    int from;
    int to;
    std::uint64_t weight;
};

namespace detail
{
using ShortestPathAdjacency = std::vector<std::vector<std::pair<std::uint64_t, int>>>;

inline std::vector<std::optional<std::uint64_t>> shortest_path_edge_distances(const ShortestPathAdjacency &graph, int source)
{
    using State = std::pair<std::uint64_t, int>;
    std::priority_queue<State, std::vector<State>, std::greater<State>> pending;
    std::vector<std::optional<std::uint64_t>> distance(graph.size());
    distance[source] = 0;
    pending.emplace(0, source);
    while (!pending.empty())
    {
        const auto [prefix, vertex] = pending.top();
        pending.pop();
        if (prefix != distance[vertex].value())
        {
            continue;
        }
        for (const auto &[weight, next] : graph[vertex])
        {
            if (weight > std::numeric_limits<std::uint64_t>::max() - prefix)
            {
                continue;
            }
            const std::uint64_t candidate = prefix + weight;
            if (!distance[next] || candidate < distance[next].value())
            {
                distance[next] = candidate;
                pending.emplace(candidate, next);
            }
        }
    }
    return distance;
}
} // namespace detail

#endif

#ifndef SIRKIT_SHORTEST_WALK_COUNTS_DEFINED
#define SIRKIT_SHORTEST_WALK_COUNTS_DEFINED

template <class Count>
struct ShortestWalkCounts
{
    std::vector<std::optional<std::uint64_t>> distance;
    std::vector<Count> count;
    std::vector<bool> infinite;
};

template <class Count = std::uint64_t>
ShortestWalkCounts<Count> count_shortest_walks(
    int n, std::span<const ShortestPathEdge> edges, int source, bool undirected = false)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n)
    {
        throw std::out_of_range("shortest walk counts: source out of range");
    }
#endif
    detail::ShortestPathAdjacency graph(n);
    bool unit = true;
    for (const auto &[u, v, weight] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("shortest walk counts: endpoint out of range");
        }
#endif
        unit &= weight == 1;
        graph[u].emplace_back(weight, v);
        if (undirected && u != v)
        {
            graph[v].emplace_back(weight, u);
        }
    }

    ShortestWalkCounts<Count> result{{}, std::vector<Count>(n, Count{0}), std::vector<bool>(n)};
    result.count[source] = Count{1};
    std::vector<int> pending;
    pending.reserve(n);
    if (unit)
    {
        result.distance.resize(n);
        result.distance[source] = 0;
        pending.push_back(source);
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const int u = pending[i];
            const auto next = *result.distance[u] + 1;
            for (const auto &[weight, v] : graph[u])
            {
                if (!result.distance[v])
                {
                    result.distance[v] = next;
                    pending.push_back(v);
                }
                if (*result.distance[v] == next)
                {
                    result.count[v] += result.count[u];
                }
            }
        }
        return result;
    }

    result.distance = detail::shortest_path_edge_distances(graph, source);
    std::vector<std::vector<int>> tight(n);
    std::vector<std::size_t> incoming(n);
    for (int u = 0; u < n; ++u)
    {
        const auto prefix = result.distance[u];
        if (!prefix)
        {
            continue;
        }
        for (const auto &[weight, v] : graph[u])
        {
            const auto finish = result.distance[v];
            if (finish && *prefix <= *finish && weight == *finish - *prefix)
            {
                tight[u].push_back(v);
                ++incoming[v];
            }
        }
    }

    for (int u = 0; u < n; ++u)
    {
        if (result.distance[u] && incoming[u] == 0)
        {
            pending.push_back(u);
        }
    }
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const int u = pending[i];
        for (const int v : tight[u])
        {
            result.count[v] += result.count[u];
            if (--incoming[v] == 0)
            {
                pending.push_back(v);
            }
        }
    }
    // 入次数が残る頂点へは、最短な歩道の途中で重み0の閉路を繰り返せる。
    for (int u = 0; u < n; ++u)
    {
        if (incoming[u] != 0)
        {
            result.infinite[u] = true;
            result.count[u] = Count{0};
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
