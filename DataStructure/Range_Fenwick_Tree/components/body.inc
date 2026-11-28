#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_RANGE_FENWICK_TREE_DEFINED
#define SIRKIT_RANGE_FENWICK_TREE_DEFINED

template <std::integral T = std::int64_t>
    requires(!std::same_as<T, bool> && sizeof(T) <= sizeof(std::uint64_t))
class RangeFenwickTree
{
    using Storage = std::uint64_t;

    struct Difference
    {
        Storage delta = 0;
        Storage weighted = 0;

        Difference &operator+=(const Difference &other)
        {
            delta += other.delta;
            weighted += other.weighted;
            return *this;
        }

        friend Difference operator-(const Difference &left, const Difference &right)
        {
            return {left.delta - right.delta, left.weighted - right.weighted};
        }
    };

public:
    explicit RangeFenwickTree(int n = 0) : differences_(n)
    {
    }

    explicit RangeFenwickTree(std::span<const T> values) : differences_(Build(values))
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

    void add(int first, int last, T delta)
    {
        CheckRange(first, last);
        if (first == last)
        {
            return;
        }
        AddRange(first, last, static_cast<Storage>(delta));
    }

    [[nodiscard]] T sum(int first, int last)
    {
        CheckRange(first, last);
        return static_cast<T>(Prefix(last) - Prefix(first));
    }

    [[nodiscard]] T get(int index)
    {
        CheckIndex(index);
        return static_cast<T>(differences_.sum(0, index + 1).delta);
    }

    void set(int index, T value)
    {
        CheckIndex(index);
        const Storage current = differences_.sum(0, index + 1).delta;
        AddRange(index, index + 1, static_cast<Storage>(value) - current);
    }

private:
    static Difference Combine(Storage delta, std::size_t index)
    {
        return {delta, delta * index};
    }

    static std::vector<Difference> Build(std::span<const T> values)
    {
        std::vector<Difference> result;
        detail::CheckedFenwickStorageSize(values.size(), result.max_size());
        result.resize(values.size());
        Storage previous = 0;
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            const auto current = static_cast<Storage>(values[i]);
            result[i] = Combine(current - previous, i);
            previous = current;
        }
        return result;
    }

    void CheckIndex([[maybe_unused]] int index) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (index < 0 || index >= size())
        {
            throw std::out_of_range("RangeFenwickTree: index");
        }
#endif
    }

    void CheckRange([[maybe_unused]] int first, [[maybe_unused]] int last) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (first < 0 || first > last || last > size())
        {
            throw std::out_of_range("RangeFenwickTree: range");
        }
#endif
    }

    void AddRange(int first, int last, Storage delta)
    {
        differences_.add(first, Combine(delta, static_cast<std::size_t>(first)));
        if (last < size())
        {
            differences_.add(last, Combine(Storage{0} - delta, static_cast<std::size_t>(last)));
        }
    }

    Storage Prefix(int count)
    {
        const auto prefix = differences_.sum(0, count);
        return prefix.delta * static_cast<Storage>(count) - prefix.weighted;
    }

    // Difference stores a signed change at a position, represented modulo 2^64.
    fenwick_tree<Difference> differences_;
};

#endif
