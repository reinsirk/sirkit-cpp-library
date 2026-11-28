// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_BOUNDED_NONDECREASING_SEQUENCES_DEFINED
#define SIRKIT_BOUNDED_NONDECREASING_SEQUENCES_DEFINED
namespace reinsirk::detail
{
class BoundedSequenceCounter
{
    using Residue = std::uint32_t;
    using Values = std::vector<Residue>;
    static constexpr auto kMod = NTT<>::modulus();
    static constexpr std::size_t kDirectExtent = 16;

    std::span<const std::uint32_t> lower_;
    std::span<const std::uint32_t> upper_;
    const std::uint32_t origin_;
    Values rows_;
    Values columns_;
    Values factorial_;
    Values inverse_factorial_;
    NTT<> ntt_;

    static Residue add(Residue a, Residue b) noexcept
    {
        const auto sum = a + b;
        return sum < kMod ? sum : sum - kMod;
    }

    static Residue multiply(Residue a, Residue b) noexcept
    {
        return static_cast<Residue>(std::uint64_t{a} * b % kMod);
    }

    static Residue inverse(Residue a) noexcept
    {
        Residue result = 1;
        for (auto exponent = kMod - 2; exponent != 0; exponent >>= 1)
        {
            if ((exponent & 1U) != 0)
            {
                result = multiply(result, a);
            }
            a = multiply(a, a);
        }
        return result;
    }

    [[nodiscard]] Values transfer(std::span<const Residue> parallel, std::span<const Residue> perpendicular)
    {
        const auto distance = perpendicular.size() - 1;
        Values kernel(parallel.size());
        for (std::size_t i = 0; i < kernel.size(); ++i)
        {
            kernel[i] = multiply(factorial_[distance + i], multiply(inverse_factorial_[distance], inverse_factorial_[i]));
        }
        // 向かいの辺からの寄与は二項係数の列との畳み込み。
        auto result = ntt_.convolution_normalized(Values(parallel.begin(), parallel.end()), std::move(kernel));
        result.resize(parallel.size());
        Values weighted(perpendicular.size());
        for (std::size_t i = 0; i < weighted.size(); ++i)
        {
            weighted[i] = multiply(perpendicular[i], inverse_factorial_[distance - i]);
        }
        // 隣の辺からの寄与は、入力と出力を逆階乗で重み付けしてから階乗を畳み込む。
        const auto cross = ntt_.convolution_normalized(std::move(weighted),
                                                       Values(factorial_.begin(), factorial_.begin() + distance + parallel.size()));
        for (std::size_t i = 0; i < result.size(); ++i)
        {
            result[i] = add(result[i], multiply(cross[distance + i], inverse_factorial_[i]));
        }
        return result;
    }

    void direct(std::size_t row_begin, std::size_t row_end, std::size_t column_begin, std::size_t column_end)
    {
        for (auto i = row_begin; i < row_end; ++i)
        {
            auto prefix = rows_[i];
            for (auto j = column_begin; j < column_end; ++j)
            {
                const auto value = origin_ + static_cast<std::uint32_t>(j);
                if (lower_[i] <= value && value < upper_[i])
                {
                    prefix = add(prefix, columns_[j]);
                }
                columns_[j] = prefix;
            }
            rows_[i] = prefix;
        }
    }

    void divide(std::size_t row, std::size_t column, std::size_t side)
    {
        if (row >= rows_.size() || column >= columns_.size())
        {
            return;
        }
        const auto height = std::min(side, rows_.size() - row);
        const auto width = std::min(side, columns_.size() - column);
        const auto last_row = row + height - 1;
        const auto first_value = origin_ + static_cast<std::uint32_t>(column);
        const auto end_value = first_value + static_cast<std::uint32_t>(width);
        auto top = std::span<Residue>(columns_).subspan(column, width);
        auto left = std::span<Residue>(rows_).subspan(row, height);
        if (lower_[row] >= end_value || upper_[last_row] <= first_value)
        {
            std::ranges::fill(top, left.back());
        }
        else if (std::min(height, width) <= kDirectExtent)
        {
            direct(row, row + height, column, column + width);
        }
        else if (lower_[last_row] <= first_value && end_value <= upper_[row])
        {
            const auto bottom = transfer(top, left);
            const auto right = transfer(left, top);
            std::ranges::copy(bottom, top.begin());
            std::ranges::copy(right, left.begin());
        }
        else
        {
            const auto half = side / 2;
            divide(row, column, half);
            divide(row, column + half, half);
            divide(row + half, column, half);
            divide(row + half, column + half, half);
        }
    }

public:
    BoundedSequenceCounter(std::span<const std::uint32_t> lower, std::span<const std::uint32_t> upper, std::size_t width)
        : lower_(lower), upper_(upper), origin_(lower.front()), rows_(lower.size()), columns_(width, 1),
          factorial_(rows_.size() + width), inverse_factorial_(factorial_.size())
    {
        factorial_[0] = 1;
        for (std::size_t i = 1; i < factorial_.size(); ++i)
        {
            factorial_[i] = multiply(factorial_[i - 1], static_cast<Residue>(i));
        }
        inverse_factorial_.back() = inverse(factorial_.back());
        for (std::size_t i = factorial_.size() - 1; i > 0; --i)
        {
            inverse_factorial_[i - 1] = multiply(inverse_factorial_[i], static_cast<Residue>(i));
        }
    }

    [[nodiscard]] Residue run()
    {
        divide(0, 0, std::bit_ceil(std::max(rows_.size(), columns_.size())));
        return columns_.back();
    }
};
}

inline std::uint32_t count_bounded_nondecreasing_sequences(std::span<const std::uint32_t> lower,
                                                           std::span<const std::uint32_t> upper)
{
    constexpr std::size_t kCapacity = std::size_t{1} << 21;
    if (lower.size() != upper.size())
    {
        throw std::invalid_argument("count_bounded_nondecreasing_sequences: size mismatch");
    }
    const auto n = lower.size();
    if (n == 0)
    {
        return 1;
    }
    // 広義単調増加なので、下限の累積最大と上限の後ろからの累積最小で置き換えられる。
    std::vector<std::uint32_t> minimum(n), maximum(n);
    std::partial_sum(lower.begin(), lower.end(), minimum.begin(), [](auto a, auto b)
                     {
                         return std::max(a, b);
                     });
    std::partial_sum(upper.rbegin(), upper.rend(), maximum.rbegin(), [](auto a, auto b)
                     {
                         return std::min(a, b);
                     });
    for (std::size_t i = 0; i < n; ++i)
    {
        if (minimum[i] >= maximum[i])
        {
            return 0;
        }
    }
    const auto width = maximum.back() - minimum.front();
    if (n > kCapacity || width > kCapacity - n)
    {
        throw std::length_error("count_bounded_nondecreasing_sequences: size limit exceeded");
    }
    return ::reinsirk::detail::BoundedSequenceCounter(minimum, maximum, width).run();
}
#endif
