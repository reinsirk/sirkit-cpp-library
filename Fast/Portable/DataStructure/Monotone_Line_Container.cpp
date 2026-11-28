// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Monotone_Line_Container.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/monotone_line_container.md
//
// API一覧
// MonotoneMinLineContainer / MonotoneMaxLineContainer: 符号付き64bit整数の直線包絡。
// add(slope, intercept): 直線を追加。傾きは最小化なら非増加、最大化なら非減少。償却O(1)。
// query(x): 最適な値と元の係数{value,slope,intercept}を返す。任意のxをO(log(h+1))で照会。
// query_monotone(x): xが非減少の一連の照会。addとの合計で償却O(1)。
// どちらも空の包絡ならnullopt。値がint64_tに収まらなければoverflow_error。同点の係数は任意。
// size()/empty(): 有効な直線の本数/空か。各O(1)。clear(): 空に戻し照会順もリセットする。
// reserve(n): 有効な直線n本分を確保する。hは有効な直線数。使用領域は最大保持数・予約容量に比例。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MONOTONE_LINE_CONTAINER_DEFINED
#define SIRKIT_MONOTONE_LINE_CONTAINER_DEFINED

namespace detail
{
inline constexpr std::int64_t kMonotoneLineMinimum = std::numeric_limits<std::int64_t>::min();
inline constexpr std::uint64_t kMonotoneLinePositiveLimit = std::numeric_limits<std::int64_t>::max();
inline constexpr std::uint64_t kMonotoneLineNegativeLimit = kMonotoneLinePositiveLimit + 1;

inline std::uint64_t monotone_line_magnitude(std::int64_t value) noexcept
{
    const auto converted = static_cast<std::uint64_t>(value);
    return value < 0 ? std::uint64_t{0} - converted : converted;
}

inline std::int64_t monotone_line_signed_value(std::uint64_t magnitude, bool negative)
{
    if (magnitude > (negative ? kMonotoneLineNegativeLimit : kMonotoneLinePositiveLimit))
    {
        throw std::overflow_error("MonotoneLineContainer: value out of range");
    }
    if (negative && magnitude == kMonotoneLineNegativeLimit)
    {
        return kMonotoneLineMinimum;
    }
    const auto value = static_cast<std::int64_t>(magnitude);
    return negative ? -value : value;
}

inline std::int64_t monotone_line_evaluate(std::int64_t slope, std::int64_t intercept, std::int64_t x)
{
    using Wide = std::uint64_t;
    constexpr auto kLimit = std::numeric_limits<Wide>::max();
    const Wide coefficient = monotone_line_magnitude(slope);
    const Wide coordinate = monotone_line_magnitude(x);
    if (coefficient != 0 && coordinate > kLimit / coefficient)
    {
        throw std::overflow_error("MonotoneLineContainer: value out of range");
    }
    const Wide product = coefficient * coordinate;
    const Wide offset = monotone_line_magnitude(intercept);
    const bool product_negative = (slope < 0) != (x < 0);
    const bool offset_negative = intercept < 0;
    if (product_negative != offset_negative)
    {
        return product >= offset ? monotone_line_signed_value(product - offset, product_negative)
                                 : monotone_line_signed_value(offset - product, offset_negative);
    }
    if (product > kLimit - offset)
    {
        throw std::overflow_error("MonotoneLineContainer: value out of range");
    }
    return monotone_line_signed_value(product + offset, product_negative);
}

// ceil((a-b)/d)。d>0。64bit座標の右外は空、左外は最小の座標に丸める。
inline std::optional<std::int64_t> monotone_line_first_x(std::int64_t a, std::int64_t b, std::uint64_t d) noexcept
{
    using Wide = std::uint64_t;
    if (a < b)
    {
        const Wide quotient = (static_cast<Wide>(b) - static_cast<Wide>(a)) / d;
        return quotient >= kMonotoneLineNegativeLimit ? kMonotoneLineMinimum : -static_cast<std::int64_t>(quotient);
    }
    const Wide numerator = static_cast<Wide>(a) - static_cast<Wide>(b);
    const Wide quotient = numerator / d + (numerator % d != 0);
    if (quotient > kMonotoneLinePositiveLimit)
    {
        return std::nullopt;
    }
    return static_cast<std::int64_t>(quotient);
}
}

template <bool Minimize = true>
class MonotoneLineContainer
{
public:
    struct Result
    {
        std::int64_t value;
        std::int64_t slope;
        std::int64_t intercept;
    };

    MonotoneLineContainer() = default;
    MonotoneLineContainer(const MonotoneLineContainer &) = default;
    MonotoneLineContainer(MonotoneLineContainer &&other) noexcept
    {
        swap_state(other);
    }

    MonotoneLineContainer &operator=(MonotoneLineContainer other) noexcept
    {
        swap_state(other);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return lines_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return lines_.empty();
    }

    void clear() noexcept
    {
        lines_.clear();
        last_slope_.reset();
        last_query_.reset();
        cursor_ = 0;
    }

    void reserve(std::size_t capacity)
    {
        lines_.reserve(capacity);
    }

    void add(std::int64_t slope, std::int64_t intercept)
    {
        Entry incoming{slope, intercept, detail::kMonotoneLineMinimum};
        auto keep = lines_.size();
        while (keep > 0)
        {
            const auto &previous = lines_[keep - 1];
            if (slope == previous.slope)
            {
                if (!better_intercept(intercept, previous.intercept))
                {
                    last_slope_ = slope;
                    return;
                }
            }
            else
            {
                const auto start = first_x(previous, incoming);
                if (!start)
                {
                    last_slope_ = slope;
                    return;
                }
                if (*start > previous.start)
                {
                    incoming.start = *start;
                    break;
                }
            }
            --keep;
        }
        if (keep == lines_.size())
        {
            lines_.push_back(incoming);
        }
        else
        {
            lines_[keep] = incoming;
            lines_.resize(keep + 1);
            cursor_ = std::min(cursor_, keep);
        }
        last_slope_ = slope;
    }

    [[nodiscard]] std::optional<Result> query(std::int64_t x) const
    {
        if (empty())
        {
            return std::nullopt;
        }
        const auto end = std::ranges::upper_bound(lines_, x, {}, &Entry::start);
        return eval(*std::prev(end), x);
    }

    [[nodiscard]] std::optional<Result> query_monotone(std::int64_t x)
    {
        if (empty())
        {
            last_query_ = x;
            return std::nullopt;
        }
        auto best = cursor_;
        while (best + 1 < lines_.size() && lines_[best + 1].start <= x)
        {
            ++best;
        }
        const auto answer = eval(lines_[best], x);
        cursor_ = best;
        last_query_ = x;
        return answer;
    }

private:
    struct Entry
    {
        std::int64_t slope;
        std::int64_t intercept;
        std::int64_t start;
    };

    static bool ordered_before(std::int64_t incoming, std::int64_t previous) noexcept
    {
        if constexpr (Minimize)
        {
            return incoming > previous;
        }
        else
        {
            return incoming < previous;
        }
    }

    static bool better_intercept(std::int64_t incoming, std::int64_t previous) noexcept
    {
        return ordered_before(previous, incoming);
    }

    static std::optional<std::int64_t> first_x(const Entry &previous, const Entry &incoming) noexcept
    {
        using Wide = std::uint64_t;
        if constexpr (Minimize)
        {
            const Wide difference = static_cast<Wide>(previous.slope) - static_cast<Wide>(incoming.slope);
            return detail::monotone_line_first_x(incoming.intercept, previous.intercept, difference);
        }
        else
        {
            const Wide difference = static_cast<Wide>(incoming.slope) - static_cast<Wide>(previous.slope);
            return detail::monotone_line_first_x(previous.intercept, incoming.intercept, difference);
        }
    }

    static Result eval(const Entry &line, std::int64_t x)
    {
        return {detail::monotone_line_evaluate(line.slope, line.intercept, x), line.slope, line.intercept};
    }

    void swap_state(MonotoneLineContainer &other) noexcept
    {
        lines_.swap(other.lines_);
        std::swap(last_slope_, other.last_slope_);
        std::swap(last_query_, other.last_query_);
        std::swap(cursor_, other.cursor_);
    }

    std::vector<Entry> lines_;
    std::optional<std::int64_t> last_slope_;
    std::optional<std::int64_t> last_query_;
    std::size_t cursor_ = 0;
};

using MonotoneMinLineContainer = MonotoneLineContainer<true>;
using MonotoneMaxLineContainer = MonotoneLineContainer<false>;

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
