// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Time_Dependent_Paths.cpp
// 参考（時刻を距離として扱うダイクストラ法）: https://atcoder.jp/contests/ABC192/editorial/764
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/time_dependent_paths.md
//
// API一覧
// earliest_arrival_times(n,edges,source,start_time,arrival): 始点から全頂点への最早到着時刻。
//   arrival(edge_id,time)はtime以降に乗るときの最早到着時刻、移動できなければnullopt。
// latest_departure_times(n,edges,target,deadline,departure): 期限までに終点へ着く全頂点の最遅出発時刻。
//   departure(edge_id,time)はtimeまでに着く最遅出発時刻、移動できなければnullopt。
//   edgesは0始まりの(from,to)、移動関数と結果の時刻はoptional<int64_t>。
//   早く到着すれば待てる。時刻を早めると最早到着が遅くならず、期限を延ばすと最遅出発が早まらないこと。
//   コールバック1回の費用をTとし、O(n+m log(m+1)+mT)時間、O(n+m)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <functional>
#include <numeric>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TIME_DEPENDENT_PATHS_DEFINED
#define SIRKIT_TIME_DEPENDENT_PATHS_DEFINED

namespace detail
{
template <bool Reverse, class Transition>
std::vector<std::optional<std::int64_t>> search_timetable(
    int n, std::span<const std::pair<int, int>> edges, int start, std::int64_t start_time, Transition transition)
{
    std::vector<std::size_t> offsets(static_cast<std::size_t>(n) + 1);
    for (const auto &[from, to] : edges)
    {
        ++offsets[static_cast<std::size_t>(Reverse ? to : from) + 1];
    }
    std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());
    std::vector<std::size_t> ids(edges.size());
    {
        auto write = offsets;
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto [from, to] = edges[i];
            ids[write[Reverse ? to : from]++] = i;
        }
    }

    using Entry = std::pair<std::int64_t, int>;
    using QueueOrder = std::conditional_t<Reverse, std::less<Entry>, std::greater<Entry>>;
    const std::conditional_t<Reverse, std::greater<std::int64_t>, std::less<std::int64_t>> better;
    std::priority_queue<Entry, std::vector<Entry>, QueueOrder> pending;
    std::vector<std::optional<std::int64_t>> times(n);
    times[start] = start_time;
    pending.emplace(start_time, start);
    while (!pending.empty())
    {
        const auto [time, u] = pending.top();
        pending.pop();
        if (time != *times[u])
        {
            continue;
        }
        const auto until = offsets[static_cast<std::size_t>(u) + 1];
        for (auto i = offsets[u]; i < until; ++i)
        {
            const auto id = ids[i];
            const std::optional<std::int64_t> next = std::invoke(transition, id, time);
            if (!next)
            {
                continue;
            }
            const int v = Reverse ? edges[id].first : edges[id].second;
            if (!times[v] || better(*next, *times[v]))
            {
                times[v] = *next;
                pending.emplace(*next, v);
            }
        }
    }
    return times;
}
} // namespace detail

template <class Arrival>
std::vector<std::optional<std::int64_t>> earliest_arrival_times(
    int n, std::span<const std::pair<int, int>> edges, int source, std::int64_t start_time, Arrival arrival)
{
    return detail::search_timetable<false>(n, edges, source, start_time, std::move(arrival));
}

template <class Departure>
std::vector<std::optional<std::int64_t>> latest_departure_times(
    int n, std::span<const std::pair<int, int>> edges, int target, std::int64_t deadline, Departure departure)
{
    return detail::search_timetable<true>(n, edges, target, deadline, std::move(departure));
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
