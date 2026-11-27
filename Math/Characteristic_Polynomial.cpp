// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/characteristic_polynomial.md
//
// API一覧
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
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
