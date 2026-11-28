#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_MATRIX_PENCIL_DETERMINANT_DEFINED
#define SIRKIT_MATRIX_PENCIL_DETERMINANT_DEFINED

namespace detail
{
template <std::uint32_t Mod>
class MatrixPencilReducer
{
    using Matrix = std::vector<std::vector<std::uint32_t>>;
    using Input = std::span<const std::vector<std::int64_t>>;

public:
    MatrixPencilReducer(Input constant, Input linear) : constant_(Normalize(constant)), linear_(Normalize(linear))
    {
    }

    [[nodiscard]] std::vector<std::uint32_t> Determinant()
    {
        const auto side = constant_.size();
        std::uint32_t scaling = 1;
        std::size_t degree_shift = 0;
        for (std::size_t i = 0; i < side; ++i)
        {
            auto pivot = Pivot(i);
            while (pivot.first == side)
            {
                if (degree_shift == side)
                {
                    return std::vector<std::uint32_t>(side + 1);
                }
                // 余った行は定数行。この行をx倍して、既に処理した列を消す。
                ++degree_shift;
                constant_[i].swap(linear_[i]);
                for (std::size_t column = 0; column < i; ++column)
                {
                    SubtractRow(i, column, linear_[i][column]);
                }
                pivot = Pivot(i);
            }
            if (pivot.first != i)
            {
                constant_[i].swap(constant_[pivot.first]);
                linear_[i].swap(linear_[pivot.first]);
                scaling = Subtract(0, scaling);
            }
            if (pivot.second != i)
            {
                for (std::size_t row = 0; row < side; ++row)
                {
                    std::swap(constant_[row][i], constant_[row][pivot.second]);
                    std::swap(linear_[row][i], linear_[row][pivot.second]);
                }
                scaling = Subtract(0, scaling);
            }
            const auto diagonal = linear_[i][i];
            const auto inverse = Inverse(diagonal);
            scaling = Multiply(scaling, diagonal);
            for (auto &value : constant_[i])
            {
                value = Multiply(value, inverse);
            }
            for (std::size_t j = i; j < side; ++j)
            {
                linear_[i][j] = Multiply(linear_[i][j], inverse);
            }
            for (std::size_t row = i + 1; row < side; ++row)
            {
                SubtractRow(row, i, linear_[row][i]);
            }
        }

        std::vector<std::vector<std::int64_t>> converted(side, std::vector<std::int64_t>(side));
        for (std::size_t reverse = side; reverse != 0; --reverse)
        {
            const auto i = reverse - 1;
            for (std::size_t j = 0; j < side; ++j)
            {
                auto value = Subtract(0, constant_[i][j]);
                for (std::size_t row = i + 1; row < side; ++row)
                {
                    value = Subtract(value, Multiply(linear_[i][row], static_cast<std::uint32_t>(converted[row][j])));
                }
                converted[i][j] = value;
            }
        }
        auto coefficients = characteristic_polynomial<Mod>(converted);
        coefficients.erase(coefficients.begin(), coefficients.begin() + static_cast<std::ptrdiff_t>(degree_shift));
        for (auto &coefficient : coefficients)
        {
            coefficient = Multiply(coefficient, scaling);
        }
        coefficients.resize(side + 1);
        return coefficients;
    }

private:
    [[nodiscard]] static Matrix Normalize(Input matrix)
    {
        Matrix result;
        result.reserve(matrix.size());
        for (const auto &row : matrix)
        {
            if (row.size() != matrix.size())
            {
                throw std::invalid_argument("matrix_pencil_determinant: matrix must be square");
            }
            auto &values = result.emplace_back();
            values.reserve(row.size());
            for (const auto value : row)
            {
                const auto remainder = value % static_cast<std::int64_t>(Mod);
                values.push_back(static_cast<std::uint32_t>(remainder < 0 ? remainder + Mod : remainder));
            }
        }
        return result;
    }

    [[nodiscard]] static std::uint32_t Multiply(std::uint32_t a, std::uint32_t b)
    {
        return static_cast<std::uint32_t>(std::uint64_t{a} * b % Mod);
    }

    [[nodiscard]] static std::uint32_t Subtract(std::uint32_t a, std::uint32_t b)
    {
        return a >= b ? a - b : Mod - (b - a);
    }

    [[nodiscard]] static std::uint32_t Inverse(std::uint32_t value)
    {
        std::uint32_t inverse = 1;
        for (std::uint32_t exponent = Mod - 2; exponent != 0; exponent >>= 1)
        {
            if ((exponent & 1u) != 0)
            {
                inverse = Multiply(inverse, value);
            }
            value = Multiply(value, value);
        }
        return inverse;
    }

    [[nodiscard]] std::pair<std::size_t, std::size_t> Pivot(std::size_t first) const
    {
        const auto side = linear_.size();
        for (std::size_t row = first; row < side; ++row)
        {
            for (std::size_t column = first; column < side; ++column)
            {
                if (linear_[row][column] != 0)
                {
                    return {row, column};
                }
            }
        }
        return {side, side};
    }

    void SubtractRow(std::size_t target, std::size_t pivot, std::uint32_t factor)
    {
        if (factor == 0)
        {
            return;
        }
        const auto side = constant_.size();
        for (std::size_t j = 0; j < side; ++j)
        {
            constant_[target][j] = Subtract(constant_[target][j], Multiply(constant_[pivot][j], factor));
        }
        for (std::size_t j = pivot; j < side; ++j)
        {
            linear_[target][j] = Subtract(linear_[target][j], Multiply(linear_[pivot][j], factor));
        }
    }

    Matrix constant_;
    Matrix linear_;
};
} // namespace detail

template <std::uint32_t Mod = 998244353>
[[nodiscard]] std::vector<std::uint32_t> matrix_pencil_determinant(
    std::span<const std::vector<std::int64_t>> constant,
    std::span<const std::vector<std::int64_t>> linear)
{
    static_assert(detail::characteristic_prime(Mod), "matrix_pencil_determinant: modulus must be prime");
    if (constant.size() != linear.size())
    {
        throw std::invalid_argument("matrix_pencil_determinant: matrix dimensions differ");
    }
    return detail::MatrixPencilReducer<Mod>(constant, linear).Determinant();
}

#endif
