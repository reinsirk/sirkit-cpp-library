// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Johnson_Shortest_Paths.cpp
// 参考（Johnson 法）: https://doi.org/10.1145/321992.321993
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/johnson_shortest_paths.md
//
// API一覧
// JohnsonEdge{from,to,weight}: 0始まりの端点と int64_t の重み。自己辺・多重辺可。
// for_each_johnson_distance_row(n,edges,on_row): 全点対の最短距離を始点順に1行ずつ渡す。
//   on_row(source, span<const optional<int64_t>>) の span は呼び出し中だけ有効。未到達はnullopt。
//   負閉路があれば on_row を呼ばず false。それ以外は true。
//   重みの絶対値は INT64_MAX/(4 max(1,n)) 以下。負辺可。
//   時間 O(n(n+m log(m+1)))（コールバックを除く）、追加領域 O(n+m)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
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

#ifndef SIRKIT_JOHNSON_SHORTEST_PATHS_DEFINED
#define SIRKIT_JOHNSON_SHORTEST_PATHS_DEFINED

struct JohnsonEdge
{
    int from;
    int to;
    std::int64_t weight;
};

template <typename Consumer>
bool for_each_johnson_distance_row(int n, std::span<const JohnsonEdge> edges, Consumer &&on_row)
{
    bool needs_potentials = false;
    for (const auto &[u, v, weight] : edges)
    {
        needs_potentials = needs_potentials || weight < 0;
    }

    // 全頂点に重み0で届く仮の始点を、初期値0で表す。
    std::vector<std::int64_t> potential(n);
    if (needs_potentials)
    {
        std::vector<std::int64_t> next(n);
        for (int remaining = n;; --remaining)
        {
            next = potential;
            bool improved = false;
            for (const auto &[u, v, weight] : edges)
            {
                const auto candidate = potential[u] + weight;
                if (candidate < next[v])
                {
                    next[v] = candidate;
                    improved = true;
                }
            }
            if (!improved)
            {
                break;
            }
            if (remaining == 1)
            {
                return false;
            }
            potential.swap(next);
        }
    }

    detail::ShortestPathAdjacency graph(n);
    for (const auto &[u, v, weight] : edges)
    {
        const auto nonnegative = weight + potential[u] - potential[v];
        graph[u].emplace_back(static_cast<std::uint64_t>(nonnegative), v);
    }
    std::vector<std::optional<std::int64_t>> row(n);
    for (int u = 0; u < n; ++u)
    {
        const auto reweighted = detail::shortest_path_edge_distances(graph, u);
        for (int v = 0; v < n; ++v)
        {
            if (reweighted[v])
            {
                row[v] = static_cast<std::int64_t>(*reweighted[v]) - potential[u] + potential[v];
            }
            else
            {
                row[v] = std::nullopt;
            }
        }
        std::invoke(on_row, u, std::span<const std::optional<std::int64_t>>(row));
    }
    return true;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
