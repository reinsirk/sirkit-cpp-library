#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef SIRKIT_RANGE_EQUAL_RUNS_DEFINED
#define SIRKIT_RANGE_EQUAL_RUNS_DEFINED

struct EqualRunLengths
{
    std::uint32_t length = 0;
    std::uint32_t prefix = 0;
    std::uint32_t suffix = 0;
    std::uint32_t longest = 0;
};

namespace reinsirk::detail
{
inline void ValidateEqualRunLength(std::size_t n)
{
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("RangeEqualRuns: input is too long");
    }
}

inline constexpr EqualRunLengths EmptyEqualRun() noexcept
{
    return {};
}

inline constexpr EqualRunLengths SingleEqualRunEdge(bool same) noexcept
{
    const auto length = static_cast<std::uint32_t>(same);
    return {1, length, length, length};
}

inline constexpr EqualRunLengths JoinEqualRunEdges(EqualRunLengths left, EqualRunLengths right) noexcept
{
    return {
        left.length + right.length,
        left.prefix == left.length ? left.length + right.prefix : left.prefix,
        right.suffix == right.length ? right.length + left.suffix : right.suffix,
        std::max({left.longest, right.longest, left.suffix + right.prefix}),
    };
}

inline constexpr EqualRunLengths EdgesToEqualRunLengths(EqualRunLengths edges) noexcept
{
    // 空でない値の区間では、連続する等号がk本なら対応する値はk+1個。
    return {edges.length + 1, edges.prefix + 1, edges.suffix + 1, edges.longest + 1};
}
} // namespace reinsirk::detail

template <class T>
class RangeEqualRuns
{
    static_assert(std::is_nothrow_swappable_v<T>);
    using Tree = segtree<EqualRunLengths, reinsirk::detail::JoinEqualRunEdges, reinsirk::detail::EmptyEqualRun>;
    std::vector<T> values_;
    Tree edges_;

    static std::vector<T> copy_values(std::span<const T> values)
    {
        ::reinsirk::detail::ValidateEqualRunLength(values.size());
        return std::vector<T>(values.begin(), values.end());
    }

    static std::vector<EqualRunLengths> build_edges(const std::vector<T> &values)
    {
        std::vector<EqualRunLengths> result;
        if (!values.empty())
        {
            result.reserve(values.size() - 1);
        }
        for (std::size_t i = 1; i < values.size(); ++i)
        {
            result.push_back(::reinsirk::detail::SingleEqualRunEdge(values[i - 1] == values[i]));
        }
        return result;
    }

    void check_position([[maybe_unused]] std::size_t position) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position >= size())
        {
            throw std::out_of_range("RangeEqualRuns: position");
        }
#endif
    }

public:
    RangeEqualRuns() = default;

    explicit RangeEqualRuns(std::span<const T> values) : values_(copy_values(values)), edges_(build_edges(values_))
    {
    }

    RangeEqualRuns(const RangeEqualRuns &) = default;

    RangeEqualRuns(RangeEqualRuns &&other)
        : values_(std::move(other.values_)), edges_(std::move(other.edges_))
    {
        other.values_.clear();
    }

    RangeEqualRuns &operator=(RangeEqualRuns other) noexcept
    {
        values_.swap(other.values_);
        edges_.swap(other.edges_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return values_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return values_.empty();
    }

    [[nodiscard]] T get(std::size_t position) const
    {
        check_position(position);
        return values_[position];
    }

    void set(std::size_t position, T value)
    {
        check_position(position);
        const auto &before = std::as_const(values_);
        const bool has_left = position != 0;
        const bool has_right = position + 1 != size();
        const bool joins_left = has_left && (before[position - 1] == std::as_const(value));
        const bool joins_right = has_right && (std::as_const(value) == before[position + 1]);
        if constexpr (std::is_same_v<T, bool>)
        {
            values_[position] = value;
        }
        else
        {
            using std::swap;
            swap(values_[position], value);
        }
        if (has_left)
        {
            edges_.set(static_cast<int>(position - 1), ::reinsirk::detail::SingleEqualRunEdge(joins_left));
        }
        if (has_right)
        {
            edges_.set(static_cast<int>(position), ::reinsirk::detail::SingleEqualRunEdge(joins_right));
        }
    }

    [[nodiscard]] EqualRunLengths query(std::size_t left, std::size_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right || right > size())
        {
            throw std::out_of_range("RangeEqualRuns: interval");
        }
#endif
        if (left == right)
        {
            return {};
        }
        return ::reinsirk::detail::EdgesToEqualRunLengths(edges_.prod(static_cast<int>(left), static_cast<int>(right - 1)));
    }

    [[nodiscard]] EqualRunLengths all() const
    {
        return empty() ? EqualRunLengths{} : ::reinsirk::detail::EdgesToEqualRunLengths(edges_.all_prod());
    }
};

#endif
