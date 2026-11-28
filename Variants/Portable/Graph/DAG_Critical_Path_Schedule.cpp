// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/DAG_Critical_Path_Schedule.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dag_critical_path_schedule.md
//
// API一覧
// DagScheduleEdge{from,to,duration}: 0始まりの端点と非負の uint64_t 所要時間。
// dag_critical_path_schedule(n,edges,start,finish): DAG上の最短完了時刻と余裕を求める。
//   閉路があるか始点から終点に届かなければ nullopt。最短完了時刻が uint64_t を超えれば overflow_error。
//   結果の duration は完了時刻、earliest/latest は頂点の最早・最遅時刻。両者が同じ頂点はクリティカル。
//   edge_slack[i] は辺 ID i の余裕。始点から終点へのいずれの経路にも含まれない頂点・辺は nullopt。
//   critical_path はクリティカルパス一つの辺 ID 列。時間・領域 O(n+m)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DAG_CRITICAL_PATH_SCHEDULE_DEFINED
#define SIRKIT_DAG_CRITICAL_PATH_SCHEDULE_DEFINED

struct DagScheduleEdge
{
    int from;
    int to;
    std::uint64_t duration;
};

struct DagCriticalPathSchedule
{
    std::uint64_t duration;
    std::vector<std::optional<std::uint64_t>> earliest;
    std::vector<std::optional<std::uint64_t>> latest;
    std::vector<std::optional<std::uint64_t>> edge_slack;
    std::vector<std::size_t> critical_path;
};

inline std::optional<DagCriticalPathSchedule> dag_critical_path_schedule(
    int n, std::span<const DagScheduleEdge> edges, int start, int finish)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (start < 0 || start >= n || finish < 0 || finish >= n)
    {
        throw std::out_of_range("dag_critical_path_schedule: invalid terminal");
    }
#endif
    std::vector<std::vector<std::size_t>> outgoing(n);
    std::vector<std::size_t> indegree(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v, duration] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("dag_critical_path_schedule: invalid endpoint");
        }
#endif
        outgoing[u].push_back(id);
        ++indegree[v];
    }
    std::vector<int> order;
    order.reserve(static_cast<std::size_t>(n));
    for (int u = 0; u < n; ++u)
    {
        if (indegree[u] == 0)
        {
            order.push_back(u);
        }
    }
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        for (const auto id : outgoing[order[i]])
        {
            const int v = edges[id].to;
            if (--indegree[v] == 0)
            {
                order.push_back(v);
            }
        }
    }
    if (order.size() != static_cast<std::size_t>(n))
    {
        return std::nullopt;
    }

    std::vector<bool> can_finish(n);
    can_finish[finish] = true;
    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        const int u = *it;
        for (const auto id : outgoing[u])
        {
            if (can_finish[edges[id].to])
            {
                can_finish[u] = true;
                break;
            }
        }
    }
    if (!can_finish[start])
    {
        return std::nullopt;
    }

    DagCriticalPathSchedule result{0, std::vector<std::optional<std::uint64_t>>(n), std::vector<std::optional<std::uint64_t>>(n), std::vector<std::optional<std::uint64_t>>(edges.size()), {}};
    result.earliest[start] = 0;
    for (int u : order)
    {
        if (!result.earliest[u])
        {
            continue;
        }
        const auto prefix = *result.earliest[u];
        for (const auto id : outgoing[u])
        {
            const auto [from, to, duration] = edges[id];
            if (!can_finish[to])
            {
                continue;
            }
            if (duration > std::numeric_limits<std::uint64_t>::max() - prefix)
            {
                throw std::overflow_error("dag_critical_path_schedule: duration overflows uint64_t");
            }
            const auto arrival = prefix + duration;
            auto &earliest = result.earliest[to];
            if (!earliest || arrival > *earliest)
            {
                earliest = arrival;
            }
        }
    }

    result.duration = result.earliest[finish].value();
    result.latest[finish] = result.duration;
    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        const int u = *it;
        if (!result.earliest[u])
        {
            continue;
        }
        for (const auto id : outgoing[u])
        {
            const auto [from, to, duration] = edges[id];
            if (!result.latest[to])
            {
                continue;
            }
            const auto deadline = *result.latest[to] - duration;
            auto &latest = result.latest[u];
            if (!latest || deadline < *latest)
            {
                latest = deadline;
            }
            result.edge_slack[id] = deadline - *result.earliest[u];
        }
    }

    for (int u = start; u != finish;)
    {
        const auto &arcs = outgoing[u];
        const auto id = *std::ranges::find_if(arcs, [&](std::size_t i)
                                              {
                                                  return result.edge_slack[i] == 0;
                                              });
        result.critical_path.push_back(id);
        u = edges[id].to;
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
