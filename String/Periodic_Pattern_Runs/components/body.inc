#include <cstddef>
#include <numeric>
#include <optional>
#include <string_view>
#include <vector>

#ifndef SIRKIT_PERIODIC_PATTERN_RUNS_DEFINED
#define SIRKIT_PERIODIC_PATTERN_RUNS_DEFINED

struct PeriodicPatternRun
{
    std::size_t start;
    std::optional<std::size_t> repetitions;
};

namespace reinsirk::detail
{
inline std::vector<unsigned char> FindPeriodicPatternStarts(std::string_view period, std::string_view pattern)
{
    const auto border = prefix_function(pattern);
    std::size_t phase = 0, matched = 0;
    const auto scan_next = [&]()
    {
        const char byte = period[phase];
        if (++phase == period.size())
        {
            phase = 0;
        }
        while (matched != 0 && byte != pattern[matched])
        {
            matched = border[matched - 1];
        }
        matched += byte == pattern[matched];
        if (matched == pattern.size())
        {
            matched = border[matched - 1];
            return true;
        }
        return false;
    };

    // 最初の一致の終端を調べる直前まで読み進める。周期列自体は連結しない。
    for (std::size_t i = 1; i < pattern.size(); ++i)
    {
        (void)scan_next();
    }
    std::vector<unsigned char> starts(period.size());
    for (auto &match : starts)
    {
        match = scan_next();
    }
    return starts;
}
} // namespace reinsirk::detail

[[nodiscard]] inline PeriodicPatternRun longest_periodic_pattern_run(std::string_view period, std::string_view pattern)
{
    if (pattern.empty())
    {
        return {0, std::nullopt};
    }
    if (period.empty())
    {
        return {0, 0};
    }

    const auto starts = reinsirk::detail::FindPeriodicPatternStarts(period, pattern);
    const std::size_t n = period.size();
    const std::size_t step = pattern.size() % n;
    const std::size_t wrap = n - step;
    const auto advance = [wrap, step](std::size_t position)
    {
        return position < wrap ? position + step : position - wrap;
    };

    std::size_t best = 0, best_start = 0;
    const std::size_t cycles = std::gcd(n, step);
    for (std::size_t seed = 0; seed < cycles; ++seed)
    {
        // 1回消費すると、周期上の位置はpatternの長さだけずれる。
        std::size_t gap = seed;
        while (starts[gap])
        {
            gap = advance(gap);
            if (gap == seed)
            {
                return {seed, std::nullopt};
            }
        }

        // 一致しない位置を境に一周すれば、周回の境界をまたぐ連続区間も数えられる。
        std::size_t position = gap, length = 0, start = 0;
        do
        {
            position = advance(position);
            if (starts[position])
            {
                if (length == 0)
                {
                    start = position;
                }
                ++length;
            }
            else
            {
                if (length > best || (length == best && start < best_start))
                {
                    best = length;
                    best_start = start;
                }
                length = 0;
            }
        } while (position != gap);
    }
    return {best_start, best};
}

#endif
