#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_CONCAVE_MIN_PLUS_CONVOLUTION_DEFINED
#define SIRKIT_CONCAVE_MIN_PLUS_CONVOLUTION_DEFINED
namespace reinsirk::detail
{
inline bool ConcaveSteps(std::int64_t a, std::int64_t b, std::int64_t c)
{
    const auto left_nonnegative = b >= a;
    const auto right_nonnegative = c >= b;
    if (left_nonnegative != right_nonnegative)
    {
        return left_nonnegative;
    }
    // 符号が同じなら、64ビットの剰余表現でも差分の大小は保たれる。
    const auto left = static_cast<std::uint64_t>(b) - static_cast<std::uint64_t>(a);
    const auto right = static_cast<std::uint64_t>(c) - static_cast<std::uint64_t>(b);
    return left >= right;
}

inline bool ConcaveSumFits(std::int64_t x, std::int64_t y)
{
    if (x >= 0)
    {
        return y <= std::numeric_limits<std::int64_t>::max() - x;
    }
    return y >= std::numeric_limits<std::int64_t>::min() - x;
}
}

inline std::vector<std::int64_t> min_plus_convolution_concave_arbitrary(std::span<const std::int64_t> a,
                                                                        std::span<const std::int64_t> b)
{
    if (a.empty() || b.empty())
    {
        return {};
    }
    const auto length = static_cast<std::size_t>(::reinsirk::detail::CheckedMinPlusLength(a.size(), b.size()));
#ifndef SKIP_RUNTIME_CHECKS
    for (std::size_t i = 2; i < a.size(); ++i)
    {
        if (!::reinsirk::detail::ConcaveSteps(a[i - 2], a[i - 1], a[i]))
        {
            throw std::invalid_argument("min_plus_convolution_concave_arbitrary: first sequence is not concave");
        }
    }
#endif
    const auto [amin, amax] = std::minmax_element(a.begin(), a.end());
    const auto [bmin, bmax] = std::minmax_element(b.begin(), b.end());
    if (!::reinsirk::detail::ConcaveSumFits(*amin, *bmin) || !::reinsirk::detail::ConcaveSumFits(*amax, *bmax))
    {
        throw std::overflow_error("min_plus_convolution_concave_arbitrary: sum exceeds int64_t");
    }
    std::vector<std::int64_t> result(length, std::numeric_limits<std::int64_t>::max());
    const auto width = a.size();
    const auto apply = [&](std::size_t left, std::size_t right, std::size_t first, std::size_t last)
    {
        if (first >= last)
        {
            return;
        }
        if (right - left <= 4 || last - first <= 4)
        {
            for (auto k = left; k < right; ++k)
            {
                for (auto j = first; j < last; ++j)
                {
                    result[k] = std::min(result[k], a[k - j] + b[j]);
                }
            }
            return;
        }
        // 凹列に対する最小列は、bの添字を逆順に並べると非減少になる。
        const auto cost = [&](int row, int column)
        {
            const auto k = left + static_cast<std::size_t>(row);
            const auto j = last - 1 - static_cast<std::size_t>(column);
            return a[k - j] + b[j];
        };
        const auto argmin = smawk(static_cast<int>(right - left), static_cast<int>(last - first),
                                  [&](int r, int j, int k)
                                  {
                                      return cost(r, k) < cost(r, j);
                                  });
        for (std::size_t i = 0; i < argmin.size(); ++i)
        {
            result[left + i] = std::min(result[left + i], cost(static_cast<int>(i), argmin[i]));
        }
    };
    const auto solve = [&](const auto &self, std::size_t left, std::size_t right, std::size_t lo, std::size_t hi) -> void
    {
        // b[j]が有効な出力区間は[j,j+width)。親で扱えなかった候補だけをこの区間へ割り当てる。
        const auto lower = right > width ? right - width : 0;
        apply(left, right, std::max(lo, lower), std::min(hi, left + 1));
        if (right - left > 1)
        {
            const auto middle = std::midpoint(left, right);
            self(self, left, middle, 0, lower);
            self(self, middle, right, left + 1, b.size());
        }
    };
    solve(solve, 0, length, 0, b.size());
    return result;
}
#endif
