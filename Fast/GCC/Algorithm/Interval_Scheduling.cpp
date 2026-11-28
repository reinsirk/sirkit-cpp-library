// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Interval_Scheduling.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/interval_scheduling.md
//
// API一覧
// ScheduleInterval{left,right,weight=1}: 空でない半開区間[left,right)。端点と重みはint64_t。
// interval_schedule_max_count(intervals): 重ならない区間を最大個数選び、元の添字を時刻順で返す。重みは使わない。
// interval_schedule_max_weight(intervals): 重みの総和を最大化し、総和weightと元の添字列indicesを返す。
// どちらも0個を選べる。端点で接する区間は選べる。複数解があればそのうちの一つ。
// 各O(n log(n+2))時間・O(n+1)領域。選べる重みの総和がint64_tの上限を超える場合はoverflow_error。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_INTERVAL_SCHEDULING_DEFINED
#define SIRKIT_INTERVAL_SCHEDULING_DEFINED

struct ScheduleInterval
{
    std::int64_t left;
    std::int64_t right;
    std::int64_t weight = 1;
};

struct WeightedIntervalSchedule
{
    std::int64_t weight;
    std::vector<std::size_t> indices;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::vector<std::size_t> interval_schedule_order(std::span<const ScheduleInterval> intervals)
{
    std::vector<std::size_t> order(intervals.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::ranges::sort(order, {}, [&](std::size_t i) { return intervals[i].right; });
    return order;
}
} // namespace detail

inline std::vector<std::size_t> interval_schedule_max_count(std::span<const ScheduleInterval> intervals)
{
    const auto order = detail::interval_schedule_order(intervals);
    std::vector<std::size_t> result;
    auto last = std::numeric_limits<std::int64_t>::min();
    for (const auto id : order)
    {
        const auto &interval = intervals[id];
        if (last <= interval.left)
        {
            last = interval.right;
            result.push_back(id);
        }
    }
    return result;
}

inline WeightedIntervalSchedule interval_schedule_max_weight(std::span<const ScheduleInterval> intervals)
{
    const auto order = detail::interval_schedule_order(intervals);
    std::vector<std::int64_t> best(order.size() + 1);
    std::vector<std::size_t> previous(order.size());
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        const auto &interval = intervals[order[i]];
        const auto first_conflict = std::upper_bound(
            order.begin(), order.begin() + static_cast<std::ptrdiff_t>(i), interval.left,
            [&](std::int64_t left, std::size_t id) { return left < intervals[id].right; });
        const auto prefix = static_cast<std::size_t>(first_conflict - order.begin());
        previous[i] = prefix;
        if (interval.weight > std::numeric_limits<std::int64_t>::max() - best[prefix])
        {
            throw std::overflow_error("interval_schedule_max_weight: selected weight overflows");
        }
        best[i + 1] = std::max(best[i], best[prefix] + interval.weight);
    }
    WeightedIntervalSchedule result{best.back(), {}};
    for (auto end = order.size(); end != 0;)
    {
        const auto i = end - 1;
        if (best[end] == best[i])
        {
            end = i;
        }
        else
        {
            result.indices.push_back(order[i]);
            end = previous[i];
        }
    }
    std::ranges::reverse(result.indices);
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
