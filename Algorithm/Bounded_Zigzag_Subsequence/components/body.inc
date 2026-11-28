#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_BOUNDED_ZIGZAG_SUBSEQUENCE_DEFINED
#define SIRKIT_BOUNDED_ZIGZAG_SUBSEQUENCE_DEFINED

struct BoundedZigzagSelection
{
    std::int64_t weight = 0;
    std::vector<std::size_t> indices;
};

namespace reinsirk::detail
{
struct BoundedZigzagEnd
{
    std::int64_t weight = 0;
    int index = -1;
};

using BoundedZigzagEnds = std::array<BoundedZigzagEnd, 2>;

inline constexpr BoundedZigzagEnds EmptyBoundedZigzagEnds() noexcept
{
    return {};
}

inline constexpr BoundedZigzagEnd BetterZigzagEnd(BoundedZigzagEnd left, BoundedZigzagEnd right) noexcept
{
    return left.weight >= right.weight ? left : right;
}

inline constexpr BoundedZigzagEnds JoinBoundedZigzagEnds(const BoundedZigzagEnds &left, const BoundedZigzagEnds &right) noexcept
{
    return {BetterZigzagEnd(left[0], right[0]), BetterZigzagEnd(left[1], right[1])};
}

inline constexpr std::uint64_t ZigzagOrderKey(std::int64_t value) noexcept
{
    return static_cast<std::uint64_t>(value) ^ (std::uint64_t{1} << 63);
}

inline std::int64_t ExtendZigzagWeight(std::int64_t gain, std::int64_t previous)
{
    if (gain > std::numeric_limits<std::int64_t>::max() - previous)
    {
        throw std::overflow_error("bounded_zigzag_subsequence: weight overflow");
    }
    return gain + previous;
}
} // namespace reinsirk::detail

[[nodiscard]] inline BoundedZigzagSelection bounded_zigzag_subsequence(std::span<const std::int64_t> values,
    std::span<const std::int64_t> weights, std::uint64_t min_step, std::uint64_t max_step)
{
    if (values.size() != weights.size() || min_step == 0 || min_step > max_step)
    {
        throw std::invalid_argument("bounded_zigzag_subsequence: length or step range");
    }
    const int n = ::reinsirk::detail::CheckedSegmentArrayCount(values.size(), "bounded_zigzag_subsequence: length");
    using Ends = reinsirk::detail::BoundedZigzagEnds;
    constexpr std::size_t kRise = 0;
    constexpr std::size_t kFall = 1;
    constexpr auto kLastKey = std::numeric_limits<std::uint64_t>::max();

    std::vector<std::uint64_t> keys;
    keys.reserve(values.size());
    for (const auto value : values)
    {
        keys.push_back(::reinsirk::detail::ZigzagOrderKey(value));
    }
    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

    segtree<Ends, reinsirk::detail::JoinBoundedZigzagEnds, reinsirk::detail::EmptyBoundedZigzagEnds> tree(static_cast<int>(keys.size()));
    const auto lower_rank = [&](std::uint64_t key)
    {
        return static_cast<int>(std::lower_bound(keys.begin(), keys.end(), key) - keys.begin());
    };
    const auto query = [&](std::uint64_t low, std::uint64_t high)
    {
        const int right = static_cast<int>(std::upper_bound(keys.begin(), keys.end(), high) - keys.begin());
        return tree.prod(lower_rank(low), right);
    };

    std::vector<std::array<int, 2>> parents(values.size());
    BoundedZigzagSelection answer;
    int best_end = -1;
    std::size_t best_direction = kRise;
    for (int i = 0; i < n; ++i)
    {
        const auto pos = static_cast<std::size_t>(i);
        const auto key = ::reinsirk::detail::ZigzagOrderKey(values[pos]);
        Ends previous{};
        // 符号付きの端を越えないよう、対応する符号なし区間で直前の向きを逆にする。
        if (key >= min_step)
        {
            previous[kRise] = query(key - std::min(key, max_step), key - min_step)[kFall];
        }
        if (kLastKey - key >= min_step)
        {
            previous[kFall] = query(key + min_step, key + std::min(kLastKey - key, max_step))[kRise];
        }

        const int rank = lower_rank(key);
        auto best = tree.get(rank);
        bool changed = false;
        for (std::size_t direction = 0; direction < 2; ++direction)
        {
            const auto weight = ::reinsirk::detail::ExtendZigzagWeight(weights[pos], previous[direction].weight);
            parents[pos][direction] = previous[direction].index;
            if (weight > best[direction].weight)
            {
                best[direction] = {weight, i};
                changed = true;
            }
            if (weight > answer.weight)
            {
                answer.weight = weight;
                best_end = i;
                best_direction = direction;
            }
        }
        if (changed)
        {
            tree.set(rank, best);
        }
    }
    for (int i = best_end; i >= 0; best_direction ^= 1)
    {
        const auto pos = static_cast<std::size_t>(i);
        answer.indices.push_back(pos);
        i = parents[pos][best_direction];
    }
    std::reverse(answer.indices.begin(), answer.indices.end());
    return answer;
}

#endif
