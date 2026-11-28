#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_MULTISET_PERMUTATION_DEFINED
#define SIRKIT_MULTISET_PERMUTATION_DEFINED

namespace detail
{
inline int MultisetPermutationSize(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("MultisetPermutation: too many elements");
    }
    return static_cast<int>(size);
}

inline BigInteger<> MultisetPermutationScaled(const BigInteger<> &ways, int numerator, int denominator)
{
    return (ways * BigInteger<>(numerator)).divmod(static_cast<std::uint32_t>(denominator)).first;
}

inline int MultisetPermutationOrdinal(const BigInteger<> &ways, const BigInteger<> &index, int remaining)
{
    // floor(index*remaining/ways) は残った各要素に順番を振ったときの0始まりの添字。
    const auto units = (index * BigInteger<>(remaining)) / ways;
    return static_cast<int>(units.divmod(std::numeric_limits<std::uint32_t>::max()).second);
}
} // namespace detail

template <std::totally_ordered T>
class MultisetPermutation
{
public:
    explicit MultisetPermutation(std::vector<T> values) : size_(detail::MultisetPermutationSize(values.size()))
    {
        std::sort(values.begin(), values.end());
        for (auto &&value : values)
        {
            if (symbols_.empty() || symbols_.back() != value)
            {
                symbols_.push_back(std::move(value));
                counts_.push_back(0);
            }
            ++counts_.back();
        }

        int placed = 0;
        for (const int count : counts_)
        {
            // 多項係数に C(placed+count,count) を掛ける。
            const int steps = std::min(placed, count);
            const int base = placed + count - steps;
            for (int i = 1; i <= steps; ++i)
            {
                ways_ = detail::MultisetPermutationScaled(ways_, base + i, i);
            }
            placed += count;
        }
    }

    [[nodiscard]] int size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] BigInteger<> count() const
    {
        return ways_;
    }

    [[nodiscard]] std::optional<BigInteger<>> rank(std::span<const T> values) const
    {
        if (values.size() != static_cast<std::size_t>(size_))
        {
            return std::nullopt;
        }
        auto counts = counts_;
        fenwick_tree<int> live(counts);
        auto ways = ways_;
        auto remaining = size_;
        BigInteger<> index;
        for (const auto &value : values)
        {
            const auto it = std::lower_bound(symbols_.begin(), symbols_.end(), value);
            if (it == symbols_.end() || *it != value)
            {
                return std::nullopt;
            }
            const auto symbol = static_cast<int>(it - symbols_.begin());
            auto &count = counts[static_cast<std::size_t>(symbol)];
            if (count == 0)
            {
                return std::nullopt;
            }
            index += detail::MultisetPermutationScaled(ways, live.sum(0, symbol), remaining);
            ways = detail::MultisetPermutationScaled(ways, count, remaining);
            --count;
            --remaining;
            live.add(symbol, -1);
        }
        return index;
    }

    [[nodiscard]] std::optional<std::vector<T>> unrank(BigInteger<> index) const
    {
        if (index.sign() < 0 || index >= ways_)
        {
            return std::nullopt;
        }
        auto counts = counts_;
        fenwick_tree<int> live(counts);
        auto ways = ways_;
        std::vector<T> result;
        result.reserve(static_cast<std::size_t>(size_));
        for (int remaining = size_; remaining > 0; --remaining)
        {
            const auto ordinal = detail::MultisetPermutationOrdinal(ways, index, remaining);
            const auto symbol = live.upper_bound(ordinal);
            const auto offset = static_cast<std::size_t>(symbol);
            result.push_back(symbols_[offset]);
            index -= detail::MultisetPermutationScaled(ways, live.sum(0, symbol), remaining);
            ways = detail::MultisetPermutationScaled(ways, counts[offset], remaining);
            --counts[offset];
            live.add(symbol, -1);
        }
        return result;
    }

private:
    int size_;
    std::vector<T> symbols_;
    std::vector<int> counts_;
    BigInteger<> ways_ = 1;
};

#endif
