#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_SUBARRAY_SUM_COUNTS_DEFINED
#define SIRKIT_SUBARRAY_SUM_COUNTS_DEFINED

struct SubarraySumBounds
{
    std::optional<std::int64_t> minimum_sum = std::nullopt;
    std::optional<std::int64_t> maximum_sum = std::nullopt;
    std::size_t minimum_length = 1;
    std::size_t maximum_length = std::numeric_limits<std::size_t>::max();
};

namespace reinsirk::detail
{
inline void CheckSubarraySumCountSize(std::size_t size)
{
    // 累積和には空接頭辞も入るため、座標数に1個分を追加する。
    if (size >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("count_subarrays_by_sum: too many values");
    }
}

inline std::int64_t AddSubarraySumPrefix(std::int64_t prefix, std::int64_t delta)
{
#ifndef SKIP_RUNTIME_CHECKS
    if ((delta > 0 && prefix > std::numeric_limits<std::int64_t>::max() - delta) ||
        (delta < 0 && prefix < std::numeric_limits<std::int64_t>::min() - delta))
    {
        throw std::overflow_error("count_subarrays_by_sum: prefix sum overflow");
    }
#endif
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(prefix) + static_cast<std::uint64_t>(delta));
}

inline std::uint64_t CountNonnegativeSubarraySums(const std::vector<std::int64_t> &prefix, SubarraySumBounds bounds)
{
    std::uint64_t answer = 0;
    std::size_t first = 0, stop = 0;
    for (std::size_t right = bounds.minimum_length; right < prefix.size(); ++right)
    {
        // 左端を右に動かすほど区間和が減るため、上下限で許される左端は連続する。
        if (bounds.maximum_sum)
        {
            while (first < right && prefix[right] - prefix[first] > *bounds.maximum_sum)
            {
                ++first;
            }
        }
        if (bounds.minimum_sum)
        {
            while (stop < right && prefix[right] - prefix[stop] >= *bounds.minimum_sum)
            {
                ++stop;
            }
        }
        else
        {
            stop = right;
        }
        const std::size_t first_by_length = right > bounds.maximum_length ? right - bounds.maximum_length : 0;
        const std::size_t begin = std::max(first, first_by_length);
        const std::size_t end = std::min(stop, right - bounds.minimum_length + 1);
        if (begin < end)
        {
            answer += end - begin;
        }
    }
    return answer;
}

inline int ShiftedSubarrayPrefixRank(const std::vector<std::int64_t> &keys, std::int64_t prefix,
                                    std::int64_t offset, bool upper)
{
    // 検索値 prefix-offset が型の外にある場合、全座標より手前か後ろになる。
    if (offset > 0 && prefix < std::numeric_limits<std::int64_t>::min() + offset)
    {
        return 0;
    }
    if (offset < 0 && prefix > std::numeric_limits<std::int64_t>::max() + offset)
    {
        return static_cast<int>(keys.size());
    }
    const std::int64_t key = prefix - offset;
    const auto iterator = upper ? std::ranges::upper_bound(keys, key) : std::ranges::lower_bound(keys, key);
    return static_cast<int>(iterator - keys.begin());
}

inline std::uint64_t CountSignedSubarraySums(const std::vector<std::int64_t> &prefix, SubarraySumBounds bounds)
{
    auto keys = prefix;
    std::ranges::sort(keys);
    const auto duplicates = std::ranges::unique(keys);
    keys.erase(duplicates.begin(), duplicates.end());
    const int size = static_cast<int>(keys.size());
    std::vector<int> ranks;
    ranks.reserve(prefix.size());
    for (const auto value : prefix)
    {
        const auto rank = std::ranges::lower_bound(keys, value) - keys.begin();
        ranks.push_back(static_cast<int>(rank));
    }

    fenwick_tree<std::int64_t> active(size);
    std::uint64_t answer = 0;
    for (std::size_t right = bounds.minimum_length; right < prefix.size(); ++right)
    {
        active.add(ranks[right - bounds.minimum_length], 1);
        if (right > bounds.maximum_length)
        {
            active.add(ranks[right - bounds.maximum_length - 1], -1);
        }
        const int begin = bounds.maximum_sum ? ShiftedSubarrayPrefixRank(keys, prefix[right], *bounds.maximum_sum, false) : 0;
        const int end = bounds.minimum_sum ? ShiftedSubarrayPrefixRank(keys, prefix[right], *bounds.minimum_sum, true) : size;
        answer += static_cast<std::uint64_t>(active.sum(begin, end));
    }
    return answer;
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::uint64_t count_subarrays_by_sum(std::span<const std::int64_t> values, SubarraySumBounds bounds = {})
{
#ifndef SKIP_RUNTIME_CHECKS
    if (bounds.minimum_length == 0 || bounds.minimum_length > bounds.maximum_length)
    {
        throw std::invalid_argument("count_subarrays_by_sum: invalid lengths");
    }
    if (bounds.minimum_sum && bounds.maximum_sum && *bounds.minimum_sum > *bounds.maximum_sum)
    {
        throw std::invalid_argument("count_subarrays_by_sum: invalid sums");
    }
#endif
    if (values.size() < bounds.minimum_length)
    {
        return 0;
    }
    reinsirk::detail::CheckSubarraySumCountSize(values.size());
    std::vector<std::int64_t> prefix(values.size() + 1);
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        prefix[i + 1] = reinsirk::detail::AddSubarraySumPrefix(prefix[i], values[i]);
    }

    if (std::ranges::is_sorted(prefix))
    {
        return reinsirk::detail::CountNonnegativeSubarraySums(prefix, bounds);
    }
    return reinsirk::detail::CountSignedSubarraySums(prefix, bounds);
}

#endif
