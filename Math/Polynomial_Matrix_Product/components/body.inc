#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_POLYNOMIAL_MATRIX_PRODUCT_DEFINED
#define SIRKIT_POLYNOMIAL_MATRIX_PRODUCT_DEFINED

namespace detail
{
class PolynomialMatrixProducts
{
    using Polynomial = std::vector<std::uint32_t>;
    using FlatMatrix = std::vector<std::uint32_t>;
    using Samples = std::vector<Polynomial>;
    using InputMatrix = std::vector<std::vector<std::vector<std::int64_t>>>;
    using OutputMatrix = std::vector<std::vector<std::uint32_t>>;
    static constexpr std::size_t kMaxDegree = 256;
    static constexpr auto kMod = kConsecutiveMod;

public:
    explicit PolynomialMatrixProducts(const InputMatrix &matrix) : side_(matrix.size())
    {
        for (const auto &row : matrix)
        {
            if (row.size() != side_)
            {
                throw std::invalid_argument("polynomial_matrix_interval_product: matrix must be square");
            }
            for (const auto &entry : row)
            {
                auto length = entry.size();
                while (length != 0 && Residue(entry[length - 1]) == 0)
                {
                    --length;
                }
                if (length > kMaxDegree + 1)
                {
                    throw std::length_error("polynomial_matrix_interval_product: degree exceeds 256");
                }
                auto &coefficients = entries_.emplace_back();
                coefficients.reserve(length);
                for (std::size_t i = 0; i < length; ++i)
                {
                    coefficients.push_back(Residue(entry[i]));
                }
                degree_ = std::max(degree_, std::max<std::size_t>(1, length) - 1);
            }
        }
    }

    [[nodiscard]] OutputMatrix Product(std::int64_t first, std::uint64_t count) const
    {
        const auto start = Residue(first);
        if (count == 0 || side_ == 0)
        {
            return Expand(Identity());
        }
        if (degree_ == 0)
        {
            return Expand(Power(Evaluate(start), count));
        }
        if (count < kMod)
        {
            return Expand(Interval(start, static_cast<std::uint32_t>(count)));
        }
        auto prefix = Power(Interval(start, kMod), count / kMod);
        const auto suffix = Interval(start, static_cast<std::uint32_t>(count % kMod));
        FlatMatrix workspace(entries_.size());
        Append(prefix, workspace, [&](std::size_t entry)
               {
                   return suffix[entry];
               });
        return Expand(prefix);
    }

private:
    [[nodiscard]] static std::uint32_t Residue(std::int64_t value)
    {
        const auto remainder = value % kMod;
        return static_cast<std::uint32_t>(remainder < 0 ? remainder + kMod : remainder);
    }

    [[nodiscard]] static std::uint32_t Add(std::uint32_t a, std::uint32_t b)
    {
        const auto sum = a + b;
        return sum >= kMod ? sum - kMod : sum;
    }

    [[nodiscard]] static std::uint32_t Evaluate(const Polynomial &coefficients, std::uint32_t point)
    {
        std::uint32_t value = 0;
        for (auto coefficient = coefficients.rbegin(); coefficient != coefficients.rend(); ++coefficient)
        {
            value = Add(ConsecutiveMultiply(value, point), *coefficient);
        }
        return value;
    }

    [[nodiscard]] FlatMatrix Evaluate(std::uint32_t point) const
    {
        FlatMatrix result(entries_.size());
        for (std::size_t i = 0; i < entries_.size(); ++i)
        {
            result[i] = Evaluate(entries_[i], point);
        }
        return result;
    }

    [[nodiscard]] FlatMatrix Identity() const
    {
        FlatMatrix result(entries_.size());
        for (std::size_t i = 0; i < side_; ++i)
        {
            result[i * side_ + i] = 1;
        }
        return result;
    }

    [[nodiscard]] OutputMatrix Expand(const FlatMatrix &matrix) const
    {
        OutputMatrix result(side_, std::vector<std::uint32_t>(side_));
        for (std::size_t i = 0; i < side_; ++i)
        {
            std::copy_n(matrix.data() + i * side_, side_, result[i].begin());
        }
        return result;
    }

    template <class GetRight>
    void Append(FlatMatrix &prefix, FlatMatrix &workspace, GetRight right) const
    {
        std::ranges::fill(workspace, 0);
        for (std::size_t i = 0; i < side_; ++i)
        {
            for (std::size_t k = 0; k < side_; ++k)
            {
                const auto left = prefix[i * side_ + k];
                for (std::size_t j = 0; j < side_; ++j)
                {
                    auto &entry = workspace[i * side_ + j];
                    entry = Add(entry, ConsecutiveMultiply(left, right(k * side_ + j)));
                }
            }
        }
        prefix.swap(workspace);
    }

    [[nodiscard]] FlatMatrix Power(FlatMatrix base, std::uint64_t exponent) const
    {
        auto result = Identity();
        FlatMatrix workspace(entries_.size());
        for (; exponent != 0; exponent >>= 1)
        {
            if ((exponent & 1u) != 0)
            {
                Append(result, workspace, [&](std::size_t entry)
                       {
                           return base[entry];
                       });
            }
            Append(base, workspace, [&](std::size_t entry)
                   {
                       return base[entry];
                   });
        }
        return result;
    }

    void AppendTail(FlatMatrix &prefix, std::uint32_t start, std::uint32_t count, FlatMatrix &workspace) const
    {
        FlatMatrix next(entries_.size());
        for (std::uint32_t i = 0; i < count; ++i)
        {
            const auto point = static_cast<std::uint32_t>((std::uint64_t{start} + i) % kMod);
            for (std::size_t entry = 0; entry < entries_.size(); ++entry)
            {
                next[entry] = Evaluate(entries_[entry], point);
            }
            Append(prefix, workspace, [&](std::size_t entry)
                   {
                       return next[entry];
                   });
        }
    }

    [[nodiscard]] Samples DoubleBlock(const Samples &current, std::size_t points, std::uint32_t shift) const
    {
        Samples left, right;
        left.reserve(current.size());
        right.reserve(current.size());
        for (const auto &entry : current)
        {
            left.push_back(evaluate_consecutive_polynomial(entry, 0, points));
            right.push_back(evaluate_consecutive_polynomial(entry, shift, points));
        }
        Samples next(entries_.size(), Polynomial(points));
        for (std::size_t i = 0; i < side_; ++i)
        {
            for (std::size_t k = 0; k < side_; ++k)
            {
                const auto &a = left[i * side_ + k];
                for (std::size_t j = 0; j < side_; ++j)
                {
                    const auto &b = right[k * side_ + j];
                    auto &out = next[i * side_ + j];
                    for (std::size_t point = 0; point < points; ++point)
                    {
                        out[point] = Add(out[point], ConsecutiveMultiply(a[point], b[point]));
                    }
                }
            }
        }
        return next;
    }

    [[nodiscard]] FlatMatrix Interval(std::uint32_t start, std::uint32_t count) const
    {
        auto result = Identity();
        FlatMatrix workspace(entries_.size());
        std::uint32_t block = 1;
        while (std::uint64_t{block} * block * 4 * degree_ <= count)
        {
            block *= 2;
        }
        if (block == 1)
        {
            AppendTail(result, start, count, workspace);
            return result;
        }

        const auto inverse = ConsecutiveInverse(block);
        Samples samples(entries_.size(), Polynomial(degree_ + 1));
        for (std::size_t i = 0; i <= degree_; ++i)
        {
            const auto point = static_cast<std::uint32_t>((std::uint64_t{start} + std::uint64_t{block} * i) % kMod);
            for (std::size_t entry = 0; entry < entries_.size(); ++entry)
            {
                samples[entry][i] = Evaluate(entries_[entry], point);
            }
        }
        for (std::uint32_t current = 1; current < block; current *= 2)
        {
            const auto points = 2 * current * degree_ + 1;
            const auto shift = ConsecutiveMultiply(current, inverse);
            samples = DoubleBlock(samples, points, shift); // 長さcurrentの積を、開始点をcurrentずらした積と掛ける。
        }

        const auto blocks = count / block;
        for (auto &entry : samples)
        {
            entry = evaluate_consecutive_polynomial(entry, 0, blocks);
        }
        for (std::size_t i = 0; i < blocks; ++i)
        {
            Append(result, workspace, [&](std::size_t entry)
                   {
                       return samples[entry][i];
                   });
        }
        const auto tail_start = static_cast<std::uint32_t>((std::uint64_t{start} + std::uint64_t{blocks} * block) % kMod);
        AppendTail(result, tail_start, count % block, workspace);
        return result;
    }

    std::size_t side_;
    std::size_t degree_ = 0;
    Samples entries_;
};
} // namespace detail

[[nodiscard]] inline std::vector<std::vector<std::uint32_t>> polynomial_matrix_interval_product(
    const std::vector<std::vector<std::vector<std::int64_t>>> &matrix, std::int64_t first, std::uint64_t count)
{
    return detail::PolynomialMatrixProducts(matrix).Product(first, count);
}

#endif
