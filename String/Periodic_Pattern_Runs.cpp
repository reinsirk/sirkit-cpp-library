// SPDX-License-Identifier: CC0-1.0
// 参考（周期列内の反復）: https://atcoder.jp/contests/abc135/tasks/abc135_f
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/periodic_pattern_runs.md
//
// API一覧
// PeriodicPatternRun{start,repetitions}: 周期内の0始まりの開始位置と連続回数。無限なら回数はnullopt。
// longest_periodic_pattern_run(period,pattern): periodを繰り返す列にpatternを何個連結して置けるか求める。
// 同じ最大回数ならstart最小。1回も置けない場合は{0,0}。空patternは{0,nullopt}。
// バイト一致。長さn,mに対しO(1+n+m)時間・領域。
//
// [同梱の接頭辞関数]
// prefix_function(text): 各接頭辞の真の接頭辞かつ接尾辞の最大長。O(n)時間・領域。
// find_occurrences(text,pattern): 一致の開始位置を昇順で返す。重複する出現も含む。
// 空のpatternは境界0からtext.size()までに一致。O(n+m)時間・O(m+k)領域、kは一致数。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <optional>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PREFIX_FUNCTION_DEFINED
#define SIRKIT_PREFIX_FUNCTION_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t occurrence_boundary_count(std::size_t size, std::size_t capacity)
{
    if (size >= capacity)
    {
        throw std::length_error("find_occurrences: too many occurrences");
    }
    return size + 1;
}
} // namespace detail

inline std::vector<std::size_t> prefix_function(std::string_view text)
{
    std::vector<std::size_t> length(text.size());
    for (std::size_t i = 1; i < text.size(); ++i)
    {
        auto previous = length[i - 1];
        while (previous != 0 && text[i] != text[previous])
        {
            previous = length[previous - 1];
        }
        length[i] = previous + (text[i] == text[previous]);
    }
    return length;
}

inline std::vector<std::size_t> find_occurrences(std::string_view text, std::string_view pattern)
{
    std::vector<std::size_t> result;
    if (pattern.empty())
    {
        result.resize(detail::occurrence_boundary_count(text.size(), result.max_size()));
        std::iota(result.begin(), result.end(), std::size_t{0});
        return result;
    }

    const auto border = prefix_function(pattern);
    std::size_t matched = 0;
    for (std::size_t i = 0; i < text.size(); ++i)
    {
        while (matched != 0 && text[i] != pattern[matched])
        {
            matched = border[matched - 1];
        }
        matched += text[i] == pattern[matched];
        if (matched == pattern.size())
        {
            result.push_back(i - (matched - 1));
            matched = border[matched - 1];
        }
    }
    return result;
}

#endif

#ifndef SIRKIT_PERIODIC_PATTERN_RUNS_DEFINED
#define SIRKIT_PERIODIC_PATTERN_RUNS_DEFINED

struct PeriodicPatternRun
{
    std::size_t start;
    std::optional<std::size_t> repetitions;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
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

    const auto starts = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FindPeriodicPatternStarts(period, pattern);
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

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
