#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_MAXIMUM_WEIGHT_WINDOW_DEFINED
#define SIRKIT_MAXIMUM_WEIGHT_WINDOW_DEFINED

struct WeightedWindowPoint
{
    std::int32_t x, y;
    std::int64_t weight;
};

struct WeightedWindowMaximum
{
    std::int64_t weight, left, bottom;
};

namespace detail
{
using WindowWeight = std::pair<std::int64_t, int>;

inline WindowWeight CombineWindowWeight(const WindowWeight &a, const WindowWeight &b)
{
    return std::max(a, b);
}

inline WindowWeight EmptyWindowWeight() noexcept
{
    return {std::numeric_limits<std::int64_t>::min(), -1};
}

inline WindowWeight IncreaseWindowWeight(std::int64_t delta, WindowWeight value) noexcept
{
    if (value.second >= 0)
    {
        value.first += delta;
    }
    return value;
}

inline std::int64_t CombineWindowDelta(std::int64_t a, std::int64_t b) noexcept
{
    return a + b;
}

inline std::int64_t ZeroWindowDelta() noexcept
{
    return 0;
}

inline void CheckWindowPointCount(std::size_t count)
{
    if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()) / 2)
    {
        throw std::length_error("maximum_weight_window: too many points");
    }
}
}

[[nodiscard]] inline WeightedWindowMaximum maximum_weight_window(std::span<const WeightedWindowPoint> points,
                                                                 std::uint32_t width, std::uint32_t height)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (width == 0 || height == 0)
    {
        throw std::invalid_argument("maximum_weight_window: positive width and height required");
    }
#endif
    detail::CheckWindowPointCount(points.size());
    if (points.empty())
    {
        return {0, 0, 0};
    }
    std::vector<std::int64_t> positions;
    positions.reserve(2 * points.size());
    std::uint64_t available = std::numeric_limits<std::int64_t>::max();
    std::int64_t past_last_x = std::numeric_limits<std::int64_t>::min();
    for (const auto &[x, y, weight] : points)
    {
        const auto bits = static_cast<std::uint64_t>(weight);
        const auto magnitude = weight < 0 ? 0 - bits : bits;
        if (magnitude > available)
        {
            throw std::overflow_error("maximum_weight_window: total magnitude out of range");
        }
        available -= magnitude;
        past_last_x = std::max(past_last_x, static_cast<std::int64_t>(x) + 1);
        positions.push_back(static_cast<std::int64_t>(y) - height + 1);
        positions.push_back(static_cast<std::int64_t>(y) + 1);
    }
    std::sort(positions.begin(), positions.end());
    positions.erase(std::unique(positions.begin(), positions.end()), positions.end());
    struct Event
    {
        std::int64_t x;
        int first, last;
        std::int64_t delta;
    };
    std::vector<Event> events;
    events.reserve(2 * points.size());
    const auto index = [&](std::int64_t y)
    {
        return static_cast<int>(std::lower_bound(positions.begin(), positions.end(), y) - positions.begin());
    };
    for (const auto &[x, y, weight] : points)
    {
        const auto first = index(static_cast<std::int64_t>(y) - height + 1);
        const auto last = index(static_cast<std::int64_t>(y) + 1);
        events.push_back({static_cast<std::int64_t>(x) - width + 1, first, last, weight});
        events.push_back({static_cast<std::int64_t>(x) + 1, first, last, -weight});
    }
    std::sort(events.begin(), events.end(), [](const Event &a, const Event &b)
              {
                  return a.x < b.x;
              });
    std::vector<detail::WindowWeight> initial(positions.size());
    for (int i = 0; i < static_cast<int>(initial.size()); ++i)
    {
        initial[i] = {0, i};
    }
    lazy_segtree<detail::WindowWeight, detail::CombineWindowWeight, detail::EmptyWindowWeight, std::int64_t,
                 detail::IncreaseWindowWeight, detail::CombineWindowDelta, detail::ZeroWindowDelta>
        tree(initial);
    WeightedWindowMaximum best{0, past_last_x, 0};
    for (std::size_t i = 0; i < events.size();)
    {
        const auto x = events[i].x;
        do
        {
            const auto &event = events[i++];
            tree.apply(event.first, event.last, event.delta);
        } while (i < events.size() && events[i].x == x);
        const auto [weight, bottom] = tree.all_prod();
        if (weight > best.weight)
        {
            best = {weight, x, positions[bottom]};
        }
    }
    return best;
}

#endif
