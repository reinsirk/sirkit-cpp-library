// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/K_Shortest_Simple_Paths.cpp
// 参考（Yen 法）: https://doi.org/10.1287/mnsc.17.11.712
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/k_shortest_simple_paths.md
//
// API一覧
// k_shortest_simple_paths(n,edges,source,target,k,undirected=false): 単純路を長さの小さい順に最大 k 本返す。
//   ShortestPathEdge{from,to,weight} は0始まりの端点と非負の uint64_t 重み。自己辺・多重辺可。
//   SimpleWeightedPath{distance,vertices,edges} は距離、頂点列、入力順の辺 ID 列。
//   同じ長さでも辺 ID 列の異なる単純路は別に数える。同距離での順は未規定。
//   始点と終点が同じなら長さ0の路だけ。距離が uint64_t を超える路は除外する。
//   最悪 O(kn(n+m log(m+1)+n log(k+1)+k)) 時間、O(n+m+kn) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <optional>
#include <queue>
#include <set>
#include <span>
#include <stdexcept>
#include <tuple>
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

#ifndef SIRKIT_K_SHORTEST_SIMPLE_PATHS_DEFINED
#define SIRKIT_K_SHORTEST_SIMPLE_PATHS_DEFINED

struct SimpleWeightedPath
{
    std::uint64_t distance;
    std::vector<int> vertices;
    std::vector<std::size_t> edges;
};

inline std::vector<SimpleWeightedPath> k_shortest_simple_paths(
    int n, std::span<const ShortestPathEdge> edges, int source, int target,
    std::size_t k, bool undirected = false)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n || target < 0 || target >= n)
    {
        throw std::out_of_range("k_shortest_simple_paths: invalid terminal");
    }
#endif
    if (k == 0)
    {
        return {};
    }
    struct Arc
    {
        int to;
        std::size_t id;
    };
    std::vector<std::vector<Arc>> graph(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [from, to, weight] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("k_shortest_simple_paths: invalid endpoint");
        }
#endif
        graph[from].push_back({to, id});
        if (undirected && from != to)
        {
            graph[to].push_back({from, id});
        }
    }

    std::vector<unsigned char> blocked_vertices(n), blocked_edges(edges.size());
    std::vector<std::optional<std::uint64_t>> distances(n);
    std::vector<int> previous_vertex(n);
    std::vector<std::size_t> previous_edge(n);
    using Entry = std::pair<std::uint64_t, int>;
    std::vector<Entry> pending;
    const std::greater<Entry> farther;
    auto shortest = [&](int start, std::uint64_t prefix) -> std::optional<SimpleWeightedPath>
    {
        std::ranges::fill(distances, std::nullopt);
        pending.clear();
        distances[start] = prefix;
        pending.emplace_back(prefix, start);
        while (!pending.empty())
        {
            std::pop_heap(pending.begin(), pending.end(), farther);
            const auto [distance, u] = pending.back();
            pending.pop_back();
            if (distance != *distances[u])
            {
                continue;
            }
            if (u == target)
            {
                SimpleWeightedPath result{distance, {target}, {}};
                for (int v = target; v != start;)
                {
                    result.edges.push_back(previous_edge[v]);
                    v = previous_vertex[v];
                    result.vertices.push_back(v);
                }
                std::ranges::reverse(result.vertices);
                std::ranges::reverse(result.edges);
                return result;
            }
            for (const auto &[v, id] : graph[u])
            {
                const auto weight = edges[id].weight;
                if (blocked_vertices[v] || blocked_edges[id] || weight > std::numeric_limits<std::uint64_t>::max() - distance)
                {
                    continue;
                }
                const auto next = distance + weight;
                if (!distances[v] || next < *distances[v])
                {
                    distances[v] = next;
                    previous_vertex[v] = u;
                    previous_edge[v] = id;
                    pending.emplace_back(next, v);
                    std::push_heap(pending.begin(), pending.end(), farther);
                }
            }
        }
        return std::nullopt;
    };

    struct Shorter
    {
        bool operator()(const SimpleWeightedPath &a, const SimpleWeightedPath &b) const
        {
            return std::tie(a.distance, a.edges) < std::tie(b.distance, b.edges);
        }
    };
    std::vector<SimpleWeightedPath> result;
    std::set<SimpleWeightedPath, Shorter> candidates;
    auto first = shortest(source, 0);
    if (!first)
    {
        return result;
    }
    result.push_back(std::move(*first));
    std::vector<std::size_t> common;
    while (result.size() < k)
    {
        const auto &base = result.back();
        common.clear();
        for (const auto &path : result)
        {
            const auto prefix = std::mismatch(base.edges.begin(), base.edges.end(), path.edges.begin(), path.edges.end());
            common.push_back(static_cast<std::size_t>(prefix.first - base.edges.begin()));
        }

        std::ranges::fill(blocked_vertices, 0);
        std::uint64_t prefix = 0;
        for (std::size_t i = 0; i < base.edges.size(); ++i)
        {
            // 同じ接頭辞から、採用済みのどの路とも異なる辺に進む。
            for (std::size_t j = 0; j < result.size(); ++j)
            {
                if (i <= common[j])
                {
                    blocked_edges[result[j].edges[i]] = 1;
                }
            }
            if (auto suffix = shortest(base.vertices[i], prefix))
            {
                suffix->vertices.insert(suffix->vertices.begin(), base.vertices.begin(), base.vertices.begin() + i);
                suffix->edges.insert(suffix->edges.begin(), base.edges.begin(), base.edges.begin() + i);
                candidates.insert(std::move(*suffix));
                if (candidates.size() > k - result.size())
                {
                    // 残り本数より下位の候補は最終結果に必要ない。
                    candidates.erase(std::prev(candidates.end()));
                }
            }
            for (std::size_t j = 0; j < result.size(); ++j)
            {
                if (i <= common[j])
                {
                    blocked_edges[result[j].edges[i]] = 0;
                }
            }
            blocked_vertices[base.vertices[i]] = 1;
            prefix += edges[base.edges[i]].weight;
        }
        if (candidates.empty())
        {
            break;
        }
        result.push_back(std::move(candidates.extract(candidates.begin()).value()));
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
