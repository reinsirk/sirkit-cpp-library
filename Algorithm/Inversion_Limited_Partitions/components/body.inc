#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_INVERSION_LIMITED_PARTITIONS_DEFINED
#define SIRKIT_INVERSION_LIMITED_PARTITIONS_DEFINED

namespace reinsirk::detail
{
inline void CheckInversionLimitedSize(std::size_t size)
{
    if (size >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("inversion_limited_starts: too many values");
    }
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::vector<std::size_t> inversion_limited_starts(
    std::span<const std::int64_t> values, std::uint64_t maximum_inversions)
{
    const std::size_t n = values.size();
    reinsirk::detail::CheckInversionLimitedSize(n);
    std::vector<std::size_t> starts(n);
    if (n < 2 || maximum_inversions >= static_cast<std::uint64_t>(n) * (n - 1) / 2)
    {
        return starts;
    }

    std::vector<std::int64_t> keys(values.begin(), values.end());
    std::ranges::sort(keys);
    const auto duplicate_keys = std::ranges::unique(keys);
    keys.erase(duplicate_keys.begin(), duplicate_keys.end());
    std::vector<int> ranks;
    ranks.reserve(n);
    for (const auto value : values)
    {
        ranks.push_back(static_cast<int>(std::ranges::lower_bound(keys, value) - keys.begin()));
    }

    const int count = static_cast<int>(keys.size());
    fenwick_tree<int> present(count);
    std::uint64_t inversions = 0;
    std::size_t left = 0;
    for (std::size_t right = 0; right < n; ++right)
    {
        const int entering = ranks[right];
        inversions += static_cast<std::uint64_t>(present.sum(entering + 1, count));
        present.add(entering, 1);
        while (inversions > maximum_inversions)
        {
            const int leaving = ranks[left];
            inversions -= static_cast<std::uint64_t>(present.sum(0, leaving));
            present.add(leaving, -1);
            ++left;
        }
        starts[right] = left;
    }
    return starts;
}

template <typename Count = std::uint64_t>
[[nodiscard]] Count count_inversion_limited_partitions(
    std::span<const std::int64_t> values, std::uint64_t maximum_inversions)
{
    const auto starts = inversion_limited_starts(values, maximum_inversions);
    if (starts.empty())
    {
        return Count{1};
    }

    // cumulative[k]は長さ0からk-1までの接頭辞の分割数を加えた値。
    std::vector<Count> cumulative(starts.size() + 1);
    cumulative[1] = Count{1};
    for (std::size_t length = 1; length < starts.size(); ++length)
    {
        Count ways = cumulative[length];
        ways -= cumulative[starts[length - 1]];
        cumulative[length + 1] = cumulative[length];
        cumulative[length + 1] += ways;
    }
    Count result = cumulative.back();
    result -= cumulative[starts.back()];
    return result;
}

#endif
