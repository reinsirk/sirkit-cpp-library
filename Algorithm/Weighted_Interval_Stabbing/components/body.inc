#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_WEIGHTED_INTERVAL_STABBING_DEFINED
#define SIRKIT_WEIGHTED_INTERVAL_STABBING_DEFINED

struct StabbingInterval
{
    int left;
    int right;
    std::int64_t weight;
};

struct IntervalStabbingResult
{
    std::int64_t weight;
    std::vector<int> points;
};

namespace reinsirk::detail
{
using StabbingState = std::optional<std::pair<std::int64_t, int>>;

inline StabbingState BestStabbingState(StabbingState a, StabbingState b) noexcept
{
    if (!a)
    {
        return b;
    }
    if (!b || a->first > b->first || (a->first == b->first && a->second < b->second))
    {
        return a;
    }
    return b;
}

inline StabbingState EmptyStabbingState() noexcept
{
    return std::nullopt;
}

inline std::int64_t AddStabbingWeight(std::int64_t weight, std::int64_t delta)
{
#ifndef SKIP_RUNTIME_CHECKS
    if ((delta > 0 && weight > std::numeric_limits<std::int64_t>::max() - delta) ||
        (delta < 0 && weight < std::numeric_limits<std::int64_t>::min() - delta))
    {
        throw std::overflow_error("maximize_interval_stabbing_weight: weight overflow");
    }
#endif
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(weight) + static_cast<std::uint64_t>(delta));
}

inline StabbingState ApplyStabbingWeight(std::int64_t delta, StabbingState state)
{
    if (state)
    {
        state->first = AddStabbingWeight(state->first, delta);
    }
    return state;
}

inline std::int64_t EmptyStabbingWeight() noexcept
{
    return 0;
}
} // namespace reinsirk::detail

[[nodiscard]] inline IntervalStabbingResult maximize_interval_stabbing_weight(
    int point_count, std::span<const StabbingInterval> intervals)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (point_count < 0)
    {
        throw std::invalid_argument("maximize_interval_stabbing_weight: negative size");
    }
#endif
    if (point_count == std::numeric_limits<int>::max())
    {
        throw std::length_error("maximize_interval_stabbing_weight: too many states");
    }
    const int states = point_count + 1;
    const auto stop = intervals.size();
    std::vector<std::size_t> heads(static_cast<std::size_t>(states), stop), next(stop);
    for (std::size_t i = 0; i < stop; ++i)
    {
        const auto &[left, right, weight] = intervals[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (left < 0 || right < left || right > point_count)
        {
            throw std::invalid_argument("maximize_interval_stabbing_weight: invalid interval");
        }
#endif
        if (left == right)
        {
            continue;
        }
        const auto end = static_cast<std::size_t>(right);
        next[i] = heads[end];
        heads[end] = i;
    }

    lazy_segtree<reinsirk::detail::StabbingState,
                 reinsirk::detail::BestStabbingState, reinsirk::detail::EmptyStabbingState,
                 std::int64_t, reinsirk::detail::ApplyStabbingWeight, reinsirk::detail::AddStabbingWeight,
                 reinsirk::detail::EmptyStabbingWeight> tree(states);
    tree.set(0, std::pair<std::int64_t, int>{0, 0});
    std::vector<int> previous(static_cast<std::size_t>(states));
    for (int end = 1; end <= point_count; ++end)
    {
        const auto [score, last] = *tree.all_prod();
        tree.set(end, std::pair{score, end});
        previous[static_cast<std::size_t>(end)] = last;

        // 右端に達した区間は、最後に選んだ点だけで取得の有無が決まる。
        for (auto i = heads[static_cast<std::size_t>(end)]; i != stop; i = next[i])
        {
            const auto &interval = intervals[i];
            tree.apply(interval.left + 1, end + 1, interval.weight);
        }
    }
    const auto [weight, last] = *tree.all_prod();
    IntervalStabbingResult result{weight, {}};
    for (int point = last; point != 0; point = previous[static_cast<std::size_t>(point)])
    {
        result.points.push_back(point - 1);
    }
    std::ranges::reverse(result.points);
    return result;
}

#endif
