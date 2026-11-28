#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_TARGET_SUM_SUBRECTANGLES_DEFINED
#define SIRKIT_TARGET_SUM_SUBRECTANGLES_DEFINED

namespace reinsirk::detail
{
using SubrectanglePrefixCounts = HashMap<std::int64_t, std::uint64_t>;

inline std::int64_t AddSubrectanglePrefix(std::int64_t left, std::int64_t right)
{
#ifndef SKIP_RUNTIME_CHECKS
    if ((right > 0 && left > std::numeric_limits<std::int64_t>::max() - right) ||
        (right < 0 && left < std::numeric_limits<std::int64_t>::min() - right))
    {
        throw std::overflow_error("count_subrectangles_with_sum: prefix overflow");
    }
#endif
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(left) + static_cast<std::uint64_t>(right));
}

inline std::uint64_t AddSubrectangleCount(std::uint64_t count, std::uint64_t matches)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (count > std::numeric_limits<std::uint64_t>::max() - matches)
    {
        throw std::overflow_error("count_subrectangles_with_sum: count overflow");
    }
#endif
    return count + matches;
}

inline std::uint64_t MatchingSubrectanglePrefixes(const SubrectanglePrefixCounts &counts,
                                                 std::int64_t prefix, std::int64_t target)
{
    // prefix-targetが符号付き64ビットの範囲外なら、候補にできる累積和はない。
    if ((target > 0 && prefix < std::numeric_limits<std::int64_t>::min() + target) ||
        (target < 0 && prefix > std::numeric_limits<std::int64_t>::max() + target))
    {
        return 0;
    }
    const auto it = counts.find(prefix - target);
    return it == counts.end() ? 0 : it->second;
}
} // namespace reinsirk::detail

template <typename ValueAt>
[[nodiscard]] std::uint64_t count_subrectangles_with_sum(int height, int width, std::int64_t target, ValueAt value_at)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (height < 0 || width < 0)
    {
        throw std::invalid_argument("count_subrectangles_with_sum: negative dimension");
    }
#endif
    if (height == 0 || width == 0)
    {
        return 0;
    }
    const bool transposed = height > width;
    const int rows = std::min(height, width);
    const int columns = std::max(height, width);
    const auto line_size = static_cast<std::size_t>(columns);
    std::vector<std::int64_t> line(line_size);
    reinsirk::detail::SubrectanglePrefixCounts counts;
    counts.reserve(line_size + 1);

    std::uint64_t answer = 0;
    // 短い辺の両端を固定し、長い辺の各位置で帯の和を更新する。
    for (int top = 0; top < rows; ++top)
    {
        std::ranges::fill(line, 0);
        for (int bottom = top; bottom < rows; ++bottom)
        {
            counts.clear();
            counts[0] = 1;
            std::int64_t prefix = 0;
            for (int i = 0; i < columns; ++i)
            {
                const auto [row, col] = transposed ? std::pair{i, bottom} : std::pair{bottom, i};
                const std::int64_t value = std::invoke(value_at, row, col);
                auto &sum = line[static_cast<std::size_t>(i)];
                sum = reinsirk::detail::AddSubrectanglePrefix(sum, value);
                prefix = reinsirk::detail::AddSubrectanglePrefix(prefix, sum);
                auto &frequency = counts[prefix];
                const auto matches = target == 0 ? frequency : reinsirk::detail::MatchingSubrectanglePrefixes(counts, prefix, target);
                answer = reinsirk::detail::AddSubrectangleCount(answer, matches);
                ++frequency;
            }
        }
    }
    return answer;
}

#endif
