#include <algorithm>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_FORBIDDEN_SUM_PARTITIONS_DEFINED
#define SIRKIT_FORBIDDEN_SUM_PARTITIONS_DEFINED

namespace reinsirk::detail
{
inline std::int64_t AddPartitionPrefix(std::int64_t prefix, std::int64_t value)
{
#ifndef SKIP_RUNTIME_CHECKS
    if ((value > 0 && prefix > std::numeric_limits<std::int64_t>::max() - value) ||
        (value < 0 && prefix < std::numeric_limits<std::int64_t>::min() - value))
    {
        throw std::overflow_error("count_partitions_avoiding_sums: prefix overflow");
    }
#endif
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(prefix) + static_cast<std::uint64_t>(value));
}
} // namespace reinsirk::detail

template <typename Count>
[[nodiscard]] Count count_partitions_avoiding_sums(
    std::span<const std::int64_t> values, std::span<const std::int64_t> forbidden)
{
    std::vector<std::int64_t> banned(forbidden.begin(), forbidden.end());
    std::ranges::sort(banned);
    banned.erase(std::unique(banned.begin(), banned.end()), banned.end());

    HashMap<std::int64_t, Count> by_prefix;
    by_prefix.reserve(values.size() + 1);
    by_prefix[0] = Count{1};
    Count total{1};
    Count answer{1};
    std::int64_t prefix = 0;
    for (const auto value : values)
    {
        prefix = reinsirk::detail::AddPartitionPrefix(prefix, value);
        answer = total;
        for (const auto bad_sum : banned)
        {
            // 減算結果が型の範囲外なら、該当する先行累積和も存在しない。
            if ((bad_sum > 0 && prefix < std::numeric_limits<std::int64_t>::min() + bad_sum) ||
                (bad_sum < 0 && prefix > std::numeric_limits<std::int64_t>::max() + bad_sum))
            {
                continue;
            }
            const auto past = by_prefix.find(prefix - bad_sum);
            if (past != by_prefix.end())
            {
                answer -= past->second;
            }
        }
        // 同じ累積和の切れ目はまとめて管理し、今の切れ目を最後に登録する。
        by_prefix[prefix] += answer;
        total += answer;
    }
    return answer;
}

template <typename Count>
[[nodiscard]] Count count_partitions_avoiding_sums(
    std::span<const std::int64_t> values, std::int64_t forbidden)
{
    return count_partitions_avoiding_sums<Count>(values, std::span{&forbidden, 1});
}

#endif
