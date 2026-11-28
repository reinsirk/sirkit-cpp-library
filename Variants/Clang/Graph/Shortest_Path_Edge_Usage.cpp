// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Shortest_Path_Edge_Usage.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_path_edge_usage.md
//
// API一覧
// [shortest_path_edge_usage]
// shortest_path_edge_usage(n, edges, source, target, undirected=false):
//   最短距離と、各辺を使う最短路が none / some / all のどれかを返す。
//   頂点は0始まり。辺の長さは正のuint64_t。最短距離をuint64_tで表せなければnullopt。
//   時間O((n+m)log(n+m+1))、領域O(n+m)。


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

#ifndef SIRKIT_SHORTEST_PATH_EDGE_USAGE_DEFINED
#define SIRKIT_SHORTEST_PATH_EDGE_USAGE_DEFINED

enum class ShortestPathEdgeUse : std::uint8_t
{
    none,
    some,
    all,
};

struct ShortestPathEdgeUsage
{
    std::optional<std::uint64_t> distance;
    std::vector<ShortestPathEdgeUse> edge_use;
};

inline ShortestPathEdgeUsage shortest_path_edge_usage(
    int n, std::span<const ShortestPathEdge> edges, int source, int target, bool undirected = false)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n || target < 0 || target >= n)
    {
        throw std::out_of_range("shortest_path_edge_usage terminal");
    }
#endif
    detail::ShortestPathAdjacency graph(n);
    detail::ShortestPathAdjacency reverse(undirected ? 0 : n);
    for (const auto &[from, to, weight] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("shortest_path_edge_usage endpoint");
        }
        if (weight == 0)
        {
            throw std::invalid_argument("shortest_path_edge_usage weight must be positive");
        }
#endif
        graph[from].emplace_back(weight, to);
        if (undirected)
        {
            graph[to].emplace_back(weight, from);
        }
        else
        {
            reverse[to].emplace_back(weight, from);
        }
    }

    const auto from_source = detail::shortest_path_edge_distances(graph, source);
    ShortestPathEdgeUsage result{from_source[target], std::vector<ShortestPathEdgeUse>(edges.size())};
    if (!result.distance || source == target)
    {
        return result;
    }
    const auto to_target = detail::shortest_path_edge_distances(undirected ? graph : reverse, target);
    const auto distance = result.distance.value();
    struct Event
    {
        std::uint64_t position;
        std::size_t edge;
        bool starts;
    };
    std::vector<Event> events;
    auto add_events = [&](int from, int to, std::uint64_t weight, std::size_t id)
    {
        if (!from_source[from] || !to_target[to])
        {
            return;
        }
        const auto left = from_source[from].value();
        const auto suffix = to_target[to].value();
        if (left > distance || weight > distance - left || suffix != distance - left - weight)
        {
            return;
        }
        result.edge_use[id] = ShortestPathEdgeUse::some;
        events.push_back({left, id, true});
        events.push_back({left + weight, id, false});
    };
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[from, to, weight] = edges[id];
        add_events(from, to, weight, id);
        if (undirected)
        {
            add_events(to, from, weight, id);
        }
    }
    std::ranges::sort(events, {}, &Event::position);
    std::size_t active_count = 0;
    std::size_t active_xor = 0;
    for (std::size_t i = 0; i < events.size();)
    {
        const std::uint64_t position = events[i].position;
        do
        {
            const auto &[point, id, starts] = events[i++];
            active_xor ^= id;
            if (starts)
            {
                ++active_count;
            }
            else
            {
                --active_count;
            }
        } while (i < events.size() && events[i].position == position);
        // Every shortest path crosses each distance between the two terminals.
        if (active_count == 1)
        {
            result.edge_use[active_xor] = ShortestPathEdgeUse::all;
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
