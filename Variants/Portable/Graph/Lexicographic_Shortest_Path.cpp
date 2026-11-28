// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Lexicographic_Shortest_Path.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lexicographic_shortest_path.md
//
// API一覧
// lexicographic_shortest_path(n,edges,source,target,undirected=false): 頂点列が辞書順最小の最短路。
//   ShortestPathEdge{from,to,weight} の端点は0始まり、重みは正の uint64_t。自己辺・多重辺可。
//   LexicographicShortestPath{distance,vertices,edges} の edges は入力順の辺 ID。
//   頂点列も同じなら辺 ID 列が辞書順最小。到達不能・距離の uint64_t 範囲超過は nullopt。
//   時間 O(n+m log(m+1))、領域 O(n+m)。


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

#ifndef SIRKIT_LEXICOGRAPHIC_SHORTEST_PATH_DEFINED
#define SIRKIT_LEXICOGRAPHIC_SHORTEST_PATH_DEFINED

struct LexicographicShortestPath
{
    std::uint64_t distance;
    std::vector<int> vertices;
    std::vector<std::size_t> edges;
};

inline std::optional<LexicographicShortestPath> lexicographic_shortest_path(
    int n, std::span<const ShortestPathEdge> edges, int source, int target, bool undirected = false)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n || target < 0 || target >= n)
    {
        throw std::out_of_range("lexicographic_shortest_path: invalid terminal");
    }
#endif
    struct Arc
    {
        int to;
        std::size_t id;
    };
    std::vector<std::vector<Arc>> graph(n);
    detail::ShortestPathAdjacency reverse(n);
    auto add = [&](int u, int v, std::uint64_t weight, std::size_t id)
    {
        graph[u].push_back({v, id});
        reverse[v].emplace_back(weight, u);
    };
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v, weight] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("lexicographic_shortest_path: invalid endpoint");
        }
        if (weight == 0)
        {
            throw std::invalid_argument("lexicographic_shortest_path: weight must be positive");
        }
#endif
        add(u, v, weight, id);
        if (undirected && u != v)
        {
            add(v, u, weight, id);
        }
    }
    const auto distances = detail::shortest_path_edge_distances(reverse, target);
    if (!distances[source])
    {
        return std::nullopt;
    }
    LexicographicShortestPath result{*distances[source], {source}, {}};
    int u = source;
    while (u != target)
    {
        // 正の重みにより、最短路を保つ候補のどれに進んでも終点までの距離が減る。
        const auto remaining = *distances[u];
        std::optional<std::pair<int, std::size_t>> next;
        for (const auto &[v, id] : graph[u])
        {
            const auto weight = edges[id].weight;
            if (distances[v] && weight <= remaining && *distances[v] == remaining - weight)
            {
                const auto candidate = std::pair{v, id};
                if (!next || candidate < *next)
                {
                    next = candidate;
                }
            }
        }
        const auto [v, id] = next.value();
        result.vertices.push_back(v);
        result.edges.push_back(id);
        u = v;
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
