#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_MONOTONE_VALUE_SHIFT_DEFINED
#define SIRKIT_MONOTONE_VALUE_SHIFT_DEFINED

namespace reinsirk::detail
{
inline constexpr std::uint64_t AddMonotoneValueDifferences(std::uint64_t left, std::uint64_t right) noexcept
{
    return left + right;
}

inline constexpr std::uint64_t EmptyMonotoneValueDifference() noexcept
{
    return 0;
}
} // namespace reinsirk::detail

class MonotoneValueShift
{
    using Value = std::int64_t;
    using Key = std::uint64_t;
    static constexpr Key kSignBit = Key{1} << 63;
    segtree<Key, reinsirk::detail::AddMonotoneValueDifferences, reinsirk::detail::EmptyMonotoneValueDifference> differences_;

    static constexpr Key key(Value value) noexcept
    {
        return static_cast<Key>(value) ^ kSignBit;
    }

    static constexpr Value original(Key coordinate) noexcept
    {
        return static_cast<Value>(coordinate ^ kSignBit);
    }

    static std::vector<Key> make_differences(std::span<const Value> values)
    {
        static_cast<void>(::reinsirk::detail::CheckedSegmentArrayCount(values.size(), "MonotoneValueShift: length"));
        std::vector<Key> result;
        result.reserve(values.size());
        Key previous = 0;
        for (const auto value : values)
        {
            const auto coordinate = key(value);
#ifndef SKIP_RUNTIME_CHECKS
            if (coordinate < previous)
            {
                throw std::invalid_argument("MonotoneValueShift: sorted input required");
            }
#endif
            result.push_back(coordinate - previous);
            previous = coordinate;
        }
        return result;
    }

    [[nodiscard]] Key key_at(int position) const
    {
        return differences_.prod(0, position + 1);
    }

    static void check_interval([[maybe_unused]] Value low, [[maybe_unused]] Value high)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (low > high)
        {
            throw std::invalid_argument("MonotoneValueShift: invalid value interval");
        }
#endif
    }

public:
    MonotoneValueShift() = default;

    explicit MonotoneValueShift(std::span<const Value> values) : differences_(make_differences(values))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return differences_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return differences_.empty();
    }

    [[nodiscard]] Value get(int position) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position < 0 || position >= size())
        {
            throw std::out_of_range("MonotoneValueShift: position");
        }
#endif
        return original(key_at(position));
    }

    [[nodiscard]] int lower_bound(Value value) const
    {
        const auto target = key(value);
        if (target == 0)
        {
            return 0;
        }
        return differences_.max_right(0, [target](Key prefix) { return prefix < target; });
    }

    [[nodiscard]] int upper_bound(Value value) const
    {
        const auto target = key(value);
        return differences_.max_right(0, [target](Key prefix) { return prefix <= target; });
    }

    [[nodiscard]] int count_between(Value low, Value high) const
    {
        check_interval(low, high);
        return upper_bound(high) - lower_bound(low);
    }

    void shift_between(Value low, Value high, Value delta)
    {
        check_interval(low, high);
        if (delta == 0)
        {
            return;
        }
        const int left = lower_bound(low);
        const int right = upper_bound(high);
        if (left == right)
        {
            return;
        }
        const auto offset = static_cast<Key>(delta);
#ifndef SKIP_RUNTIME_CHECKS
        if (delta > 0)
        {
            const auto edge = key_at(right - 1);
            if (edge > std::numeric_limits<Key>::max() - offset)
            {
                throw std::overflow_error("MonotoneValueShift: positive overflow");
            }
            if (right != size() && edge + offset > key_at(right))
            {
                throw std::invalid_argument("MonotoneValueShift: crossed right neighbor");
            }
        }
        else
        {
            const auto edge = key_at(left);
            const auto magnitude = Key{0} - offset;
            if (edge < magnitude)
            {
                throw std::overflow_error("MonotoneValueShift: negative overflow");
            }
            if (left != 0 && edge - magnitude < key_at(left - 1))
            {
                throw std::invalid_argument("MonotoneValueShift: crossed left neighbor");
            }
        }
#endif
        // 選んだ区間内の差は変わらない。符号なし演算で両端を更新し終える。
        differences_.set(left, differences_.get(left) + offset);
        if (right != size())
        {
            differences_.set(right, differences_.get(right) - offset);
        }
    }

    [[nodiscard]] std::vector<Value> values() const
    {
        const auto differences = differences_.values();
        std::vector<Value> result;
        result.reserve(differences.size());
        Key coordinate = 0;
        for (const auto difference : differences)
        {
            coordinate += difference;
            result.push_back(original(coordinate));
        }
        return result;
    }
};

#endif
