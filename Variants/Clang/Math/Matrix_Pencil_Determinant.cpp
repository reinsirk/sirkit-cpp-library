// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Matrix_Pencil_Determinant.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/matrix_pencil_determinant.md
//
// API一覧
// matrix_pencil_determinant<Mod=998244353>(constant,linear): det(constant+x*linear)の係数を定数項から返す。
// 両入力は同じサイズnの正方行列。負のint64_t係数も指定可。Modはuint32_tの素数。
// 返り値はn+1個のuint32_t。行列式が恒等的に0なら全項0、空行列なら{1}。
// 線形係数行列が特異でも可。形状不一致はinvalid_argument。O(n³+n log Mod+1)時間・O(n²+1)領域。
//
// [同梱の特性多項式]
// characteristic_polynomial<Mod=998244353>(matrix): det(xI-A)の係数を定数項から返す。
// Modは32ビット符号なしの素数。符号付き64ビットの正方行列を受け取る。空行列は{1}。
// 入力は変更しない。n×n行列でO(1+n³+n log Mod)時間・O(1+n²)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#include <algorithm>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CHARACTERISTIC_POLYNOMIAL_DEFINED
#define SIRKIT_CHARACTERISTIC_POLYNOMIAL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
consteval bool characteristic_prime(std::uint32_t n)
{
    if (n % 2 == 0 || n < 2)
    {
        return n == 2;
    }
    for (std::uint32_t d = 3; d <= n / d; d += 2)
    {
        if (n % d == 0)
        {
            return false;
        }
    }
    return true;
}
} // namespace detail

template <std::uint32_t Mod = 998244353>
std::vector<std::uint32_t> characteristic_polynomial(std::span<const std::vector<std::int64_t>> matrix)
{
    static_assert(detail::characteristic_prime(Mod), "characteristic_polynomial: please specify a prime modulus");
    const std::size_t n = matrix.size();
    const auto multiply = [](std::uint32_t a, std::uint32_t b)
    {
        return static_cast<std::uint32_t>(std::uint64_t{a} * b % Mod);
    };
    const auto add = [](std::uint32_t a, std::uint32_t b)
    {
        return a < Mod - b ? a + b : a - (Mod - b);
    };
    const auto subtract = [](std::uint32_t a, std::uint32_t b)
    {
        return a >= b ? a - b : Mod - (b - a);
    };
    const auto inverse = [&](std::uint32_t base)
    {
        std::uint32_t value = 1;
        for (std::uint32_t exp = Mod - 2; exp != 0; exp >>= 1)
        {
            if (exp & 1)
            {
                value = multiply(value, base);
            }
            base = multiply(base, base);
        }
        return value;
    };
    std::vector<std::vector<std::uint32_t>> h;
    h.reserve(n);
    for (const auto &row : matrix)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (row.size() != n)
        {
            throw std::invalid_argument("characteristic_polynomial: matrix must be square");
        }
#endif
        auto &dest = h.emplace_back();
        dest.reserve(n);
        for (std::int64_t x : row)
        {
            const std::int64_t residue = x % static_cast<std::int64_t>(Mod);
            dest.push_back(static_cast<std::uint32_t>(residue < 0 ? residue + Mod : residue));
        }
    }

    // 行操作の逆変換を列に施して相似を保ち、下Hessenberg部分を消去する。
    for (std::size_t col = 0; col + 2 < n; ++col)
    {
        const std::size_t next = col + 1;
        std::size_t pivot = next;
        while (pivot < n && h[pivot][col] == 0)
        {
            ++pivot;
        }
        if (pivot == n)
        {
            continue;
        }
        if (pivot != next)
        {
            h[pivot].swap(h[next]);
            for (auto &row : h)
            {
                std::swap(row[pivot], row[next]);
            }
        }
        const std::uint32_t inv = inverse(h[next][col]);
        for (std::size_t row = next + 1; row < n; ++row)
        {
            if (h[row][col] == 0)
            {
                continue;
            }
            const auto scale = multiply(h[row][col], inv);
            h[row][col] = 0;
            for (std::size_t j = next; j < n; ++j)
            {
                h[row][j] = subtract(h[row][j], multiply(scale, h[next][j]));
            }
            for (auto &values : h)
            {
                values[next] = add(values[next], multiply(scale, values[row]));
            }
        }
    }

    // 先頭主小行列の det(xI-H) を最終列で展開する。
    std::vector<std::vector<std::uint32_t>> polynomials(n + 1);
    polynomials[0] = {1};
    for (std::size_t last = 0; last < n; ++last)
    {
        const auto &previous = polynomials[last];
        auto &current = polynomials[last + 1];
        current.resize(last + 2);
        for (std::size_t k = 0; k <= last; ++k)
        {
            current[k] = subtract(current[k], multiply(h[last][last], previous[k]));
            current[k + 1] = previous[k];
        }
        std::uint32_t lower_product = 1;
        for (std::size_t i = last; i > 0; --i)
        {
            lower_product = multiply(lower_product, h[i][i - 1]);
            if (lower_product == 0)
            {
                break;
            }
            const auto scale = multiply(lower_product, h[i - 1][last]);
            for (std::size_t k = 0; k < i; ++k)
            {
                current[k] = subtract(current[k], multiply(scale, polynomials[i - 1][k]));
            }
        }
    }
    return std::move(polynomials.back());
}
#endif

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

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
