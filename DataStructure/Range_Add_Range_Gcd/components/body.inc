#include <concepts>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_RANGE_ADD_RANGE_GCD_DEFINED
#define SIRKIT_RANGE_ADD_RANGE_GCD_DEFINED

template <std::signed_integral T = std::int64_t>
class RangeAddRangeGcd
{
    using Unsigned = std::make_unsigned_t<T>;

    struct Differences
    {
        std::vector<Unsigned> modular;
        std::vector<Unsigned> magnitude;
    };

    static constexpr Unsigned merge_gcd(Unsigned a, Unsigned b) noexcept
    {
        return std::gcd(a, b);
    }

    static constexpr Unsigned zero() noexcept
    {
        return 0;
    }

public:
    RangeAddRangeGcd() = default;

    explicit RangeAddRangeGcd(std::span<const T> values)
        : RangeAddRangeGcd(build_differences(values))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return modular_.size();
    }

    [[nodiscard]] T get(int index)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (index < 0 || index >= size())
        {
            throw std::out_of_range("RangeAddRangeGcd::get: index");
        }
#endif
        return value_at(index);
    }

    void add(int first, int last, T delta)
    {
        check_range(first, last);
        if (first == last)
        {
            return;
        }
        const auto change = static_cast<Unsigned>(delta);
        if (last < size())
        {
            const T before = apply_delta(value_at(last - 1), change);
            magnitude_.set(last, distance(before, value_at(last)));
            modular_.add(last, static_cast<Unsigned>(Unsigned{} - change));
        }
        const T previous = first == 0 ? T{} : value_at(first - 1);
        const T after = apply_delta(value_at(first), change);
        magnitude_.set(first, distance(previous, after));
        modular_.add(first, change);
    }

    [[nodiscard]] Unsigned gcd(int first, int last)
    {
        check_range(first, last);
        if (first == last)
        {
            return 0;
        }
        return std::gcd(distance(T{}, value_at(first)), magnitude_.prod(first + 1, last));
    }

private:
    explicit RangeAddRangeGcd(const Differences &differences)
        : modular_(differences.modular), magnitude_(differences.magnitude)
    {
    }

    static Differences build_differences(std::span<const T> values)
    {
        Differences result;
        result.modular.reserve(values.size());
        result.magnitude.reserve(values.size());
        T previous{};
        for (const T value : values)
        {
            result.modular.push_back(static_cast<Unsigned>(static_cast<Unsigned>(value) - static_cast<Unsigned>(previous)));
            result.magnitude.push_back(distance(previous, value));
            previous = value;
        }
        return result;
    }

    static T apply_delta(T value, Unsigned delta) noexcept
    {
        return static_cast<T>(static_cast<Unsigned>(static_cast<Unsigned>(value) + delta));
    }

    static Unsigned distance(T a, T b) noexcept
    {
        const auto left = static_cast<Unsigned>(a);
        const auto right = static_cast<Unsigned>(b);
        return static_cast<Unsigned>(a < b ? right - left : left - right);
    }

    [[nodiscard]] T value_at(int index)
    {
        return static_cast<T>(modular_.sum(0, index + 1));
    }

    void check_range(int first, int last) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (first < 0 || first > last || last > size())
        {
            throw std::out_of_range("RangeAddRangeGcd: range");
        }
#else
        (void)first;
        (void)last;
#endif
    }

    fenwick_tree<Unsigned> modular_;
    segtree<Unsigned, merge_gcd, zero> magnitude_;
};

template <std::signed_integral T>
RangeAddRangeGcd(const std::vector<T> &) -> RangeAddRangeGcd<T>;

#endif
