#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_TRIDIAGONAL_MATRIX_POWER_DEFINED
#define SIRKIT_TRIDIAGONAL_MATRIX_POWER_DEFINED

namespace detail
{
inline constexpr std::uint32_t kTridiagonalPowerMod = kConsecutiveRecurrenceMod;

inline std::uint32_t TridiagonalResidue(std::int64_t value)
{
    const auto residue = value % kTridiagonalPowerMod;
    return static_cast<std::uint32_t>(residue < 0 ? residue + kTridiagonalPowerMod : residue);
}

inline std::uint32_t TridiagonalMultiply(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kTridiagonalPowerMod);
}

inline std::uint32_t TridiagonalAdd(std::uint32_t a, std::uint32_t b)
{
    const auto value = a + b;
    return value >= kTridiagonalPowerMod ? value - kTridiagonalPowerMod : value;
}

class TridiagonalContinuants
{
    using Polynomial = std::vector<std::uint32_t>;
    using Matrix = std::array<Polynomial, 4>;
    using Coefficients = std::span<const std::uint32_t>;

public:
    TridiagonalContinuants(std::span<const std::int64_t> diagonal, std::span<const std::int64_t> lower,
                           std::span<const std::int64_t> upper)
        : diagonal_(diagonal), lower_(lower), upper_(upper), tree_(2 * diagonal.size() - 1)
    {
        Build(0, 0, diagonal.size());
    }

    void WriteColumn(std::uint64_t exponent, Coefficients path_weights, std::span<std::uint32_t> output)
    {
        const auto side = diagonal_.size();
        auto reciprocal = ConsecutiveReciprocalSlice(std::move(tree_[0][0]), exponent - (side - 1), side, ntt_);
        WriteRange(0, 0, side, reciprocal, {}, path_weights, output);
    }

private:
    [[nodiscard]] Polynomial Product(Coefficients a, Coefficients b)
    {
        if (a.empty() || b.empty())
        {
            return {};
        }
        return ntt_.convolution_normalized(Polynomial(a.begin(), a.end()), Polynomial(b.begin(), b.end()));
    }

    [[nodiscard]] Polynomial Dot(const Polynomial &a, const Polynomial &b, const Polynomial &c, const Polynomial &d)
    {
        auto left = Product(a, b);
        auto right = Product(c, d);
        left.resize(std::max(left.size(), right.size()));
        for (std::size_t i = 0; i < right.size(); ++i)
        {
            left[i] = TridiagonalAdd(left[i], right[i]);
        }
        while (!left.empty() && left.back() == 0)
        {
            left.pop_back();
        }
        return left;
    }

    void Build(std::size_t node, std::size_t first, std::size_t last)
    {
        auto &matrix = tree_[node];
        if (last - first == 1)
        {
            matrix[0] = {1};
            const auto diagonal = TridiagonalResidue(diagonal_[first]);
            if (diagonal != 0)
            {
                matrix[0].push_back(ConsecutiveSubtract(0, diagonal));
            }
            if (first + 1 < diagonal_.size())
            {
                const auto product = TridiagonalMultiply(TridiagonalResidue(lower_[first]), TridiagonalResidue(upper_[first]));
                if (product != 0)
                {
                    matrix[1] = {0, 0, ConsecutiveSubtract(0, product)};
                }
            }
            matrix[2] = {1};
            return;
        }

        const auto middle = first + (last - first) / 2;
        const auto left = node + 1;
        const auto right = node + 2 * (middle - first);
        Build(left, first, middle);
        Build(right, middle, last);
        const auto &a = tree_[left];
        const auto &b = tree_[right];
        matrix[0] = Dot(a[0], b[0], a[1], b[2]);
        matrix[1] = Dot(a[0], b[1], a[1], b[3]);
        matrix[2] = Dot(a[2], b[0], a[3], b[2]);
        matrix[3] = Dot(a[2], b[1], a[3], b[3]);
    }

    static std::uint32_t Get(const Polynomial &coefficients, std::size_t index)
    {
        return index < coefficients.size() ? coefficients[index] : 0;
    }

    void WriteRange(std::size_t node, std::size_t first, std::size_t last,
                    Coefficients tail, Coefficients next_tail, Coefficients path_weights,
                    std::span<std::uint32_t> output)
    {
        if (last - first == 1)
        {
            output[first] = TridiagonalMultiply(path_weights[first], tail.front());
            return;
        }
        const auto middle = first + (last - first) / 2;
        const auto right = node + 2 * (middle - first);
        const auto right_size = last - middle;
        WriteRange(right, middle, last, tail.first(right_size),
                   next_tail.first(std::min(next_tail.size(), right_size + 1)), path_weights, output);

        // 左側の各係数に必要な区間だけ、右側の余因子を掛けて切り出す。
        const auto &matrix = tree_[right];
        const auto a = Product(matrix[0], tail);
        const auto b = Product(matrix[1], next_tail);
        const auto c = Product(matrix[2], tail);
        const auto d = Product(matrix[3], next_tail);
        Polynomial left(middle - first);
        Polynomial left_next(left.size() + 1);
        for (std::size_t i = 0; i < left.size(); ++i)
        {
            left[i] = TridiagonalAdd(Get(a, right_size + i), Get(b, right_size + 1 + i));
        }
        for (std::size_t i = 0; i < left_next.size(); ++i)
        {
            left_next[i] = TridiagonalAdd(Get(c, right_size - 1 + i), Get(d, right_size + i));
        }
        WriteRange(node + 1, first, middle, left, left_next, path_weights, output);
    }

    std::span<const std::int64_t> diagonal_;
    std::span<const std::int64_t> lower_;
    std::span<const std::int64_t> upper_;
    std::vector<Matrix> tree_;
    NTT<> ntt_;
};
} // namespace detail

[[nodiscard]] inline std::vector<std::uint32_t> tridiagonal_power_first_column(
    std::uint64_t exponent, std::span<const std::int64_t> lower, std::span<const std::int64_t> diagonal,
    std::span<const std::int64_t> upper)
{
    const auto side = diagonal.size();
    const auto off_diagonal = side == 0 ? 0 : side - 1;
    if (lower.size() != off_diagonal || upper.size() != off_diagonal)
    {
        throw std::invalid_argument("tridiagonal_power_first_column: diagonal sizes differ");
    }
    if (side == 0)
    {
        return {};
    }
    constexpr std::size_t kMaxSide = std::size_t{1} << 20;
    auto active = exponent < side ? static_cast<std::size_t>(exponent) + 1 : side;
    if (active > kMaxSide)
    {
        throw std::length_error("tridiagonal_power_first_column: too many active rows");
    }
    std::vector<std::uint32_t> result(side);
    if (exponent == 0)
    {
        result[0] = 1;
        return result;
    }
    std::vector<std::uint32_t> paths(active, 1);
    for (std::size_t i = 1; i < active; ++i)
    {
        const auto edge = detail::TridiagonalResidue(lower[i - 1]);
        if (edge == 0)
        {
            active = i; // この先の行には第0列から到達できない。
            break;
        }
        paths[i] = detail::TridiagonalMultiply(paths[i - 1], edge);
    }
    detail::TridiagonalContinuants system(diagonal.first(active), lower.first(active - 1), upper.first(active - 1));
    system.WriteColumn(exponent, paths, result);
    return result;
}

#endif
