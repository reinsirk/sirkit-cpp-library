// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Two_Nearest_Source_Groups.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/two_nearest_source_groups.md
//
// API一覧
// ShortestPathEdge{from,to,weight}: 重みが非負の uint64_t の辺。
// GroupedShortestPathSource{vertex,group}: 始点の頂点 ID とグループ ID。
// two_nearest_source_groups(n,edges,sources,undirected=false): 各頂点へ近い始点の上位2グループ。
//   各要素は距離順の optional<SourceGroupDistance> の長さ2の配列。
//   SourceGroupDistance{distance,source,group} は距離、始点の頂点 ID、グループ ID。
//   頂点は0始まり。グループ ID は任意の int。距離が同じ候補の順は未規定。
//   未到達・距離が uint64_t を超える候補は除く。始点数 s、x=m+s とする。
//   時間 O(n+x log(x+1))、全辺の重みが1なら O(n+x)。領域 O(n+x)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <type_traits>
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

#ifndef SIRKIT_TWO_NEAREST_SOURCE_GROUPS_DEFINED
#define SIRKIT_TWO_NEAREST_SOURCE_GROUPS_DEFINED

struct GroupedShortestPathSource
{
    int vertex;
    int group;
};

struct SourceGroupDistance
{
    std::uint64_t distance;
    int source;
    int group;
};

namespace detail
{
template <bool Unit>
auto search_two_source_groups(const ShortestPathAdjacency &graph, std::span<const GroupedShortestPathSource> sources)
{
    struct State
    {
        std::uint64_t distance;
        int vertex;
        int source;
        int group;
    };
    struct Farther
    {
        bool operator()(const State &a, const State &b) const noexcept
        {
            return a.distance > b.distance;
        }
    };
    using Queue = std::conditional_t<Unit, std::queue<State>, std::priority_queue<State, std::vector<State>, Farther>>;
    Queue pending;
    for (const auto &[vertex, group] : sources)
    {
        pending.emplace(0, vertex, vertex, group);
    }

    std::vector<std::array<std::optional<SourceGroupDistance>, 2>> result(graph.size());
    while (!pending.empty())
    {
        const auto [distance, vertex, source, group] = [&]() -> State
        {
            if constexpr (Unit)
            {
                return pending.front();
            }
            else
            {
                return pending.top();
            }
        }();
        pending.pop();
        auto &[first, second] = result[vertex];
        if (second || (first && first->group == group))
        {
            continue;
        }
        (first ? second : first) = SourceGroupDistance{distance, source, group};
        for (const auto &[weight, next] : graph[vertex])
        {
            if constexpr (!Unit)
            {
                if (weight > std::numeric_limits<std::uint64_t>::max() - distance)
                {
                    continue;
                }
            }
            pending.emplace(distance + weight, next, source, group);
        }
    }
    return result;
}
} // namespace detail

inline auto two_nearest_source_groups(
    int n, std::span<const ShortestPathEdge> edges, std::span<const GroupedShortestPathSource> sources,
    bool undirected = false)
{
    detail::ShortestPathAdjacency graph(n);
    bool unit = true;
    for (const auto &[from, to, weight] : edges)
    {
        graph[from].emplace_back(weight, to);
        if (undirected)
        {
            graph[to].emplace_back(weight, from);
        }
        unit = unit && weight == 1;
    }
    return unit ? detail::search_two_source_groups<true>(graph, sources)
                : detail::search_two_source_groups<false>(graph, sources);
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
