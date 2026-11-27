// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Pfaffian.cpp
// 参考: M. Wimmer, Efficient numerical computation of the Pfaffian for dense and banded skew-symmetric matrices
// (https://arxiv.org/abs/1102.3440) の合同変換。有限体上のコードは独自実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pfaffian.md
//
// API一覧
// pfaffian_mod<Prime=998244353>(matrix): 素数Primeを法とする交代行列のパフィアンを求める。
// D×Dのvector<vector<uint32_t>>を渡す。各要素は[0,Prime)、対角は0、A[i][j]=-A[j][i] mod Prime。
// D=0なら1、奇数なら0。通常版で形・値・交代条件が不正ならinvalid_argument。
// O(D³)時間、行列を含めO(D²)領域。元の行列を使わない場合はstd::moveで渡せる。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PFAFFIAN_DEFINED
#define SIRKIT_PFAFFIAN_DEFINED

namespace detail
{
consteval bool PfaffianPrime(std::uint32_t value)
{
    if (value % 2 == 0)
    {
        return value == 2;
    }
    for (std::uint32_t divisor = 3; divisor <= value / divisor; divisor += 2)
    {
        if (value % divisor == 0)
        {
            return false;
        }
    }
    return value > 1;
}
} // namespace detail

template <std::uint32_t Prime = 998244353>
std::uint32_t pfaffian_mod(std::vector<std::vector<std::uint32_t>> matrix)
{
    static_assert(detail::PfaffianPrime(Prime), "pfaffian_mod: modulus must be prime");
    const auto n = matrix.size();
    const auto negate = [](std::uint32_t value) -> std::uint32_t
    {
        return value == 0 ? 0 : Prime - value;
    };
    if (n % 2 != 0)
    {
        return 0;
    }

    const auto multiply = [](std::uint32_t a, std::uint32_t b)
    {
        return static_cast<std::uint32_t>((std::uint64_t{a} * b) % Prime);
    };
    std::uint32_t result = 1;
    for (std::size_t first = 0; first < n; first += 2)
    {
        const std::size_t second = first + 1;
        std::size_t pivot_column = second;
        while (pivot_column < n && matrix[first][pivot_column] == 0)
        {
            ++pivot_column;
        }
        if (pivot_column == n)
        {
            return 0;
        }
        if (pivot_column != second)
        {
            std::swap(matrix[second], matrix[pivot_column]);
            for (auto &row : matrix)
            {
                std::swap(row[second], row[pivot_column]);
            }
            result = negate(result);
        }

        const auto &top = matrix[first];
        const auto &bottom = matrix[second];
        const auto pivot = top[second];
        result = multiply(result, pivot);
        std::uint32_t inverse = 1;
        for (std::uint32_t base = pivot, power = Prime - 2; power != 0; power >>= 1)
        {
            if ((power & 1u) != 0)
            {
                inverse = multiply(inverse, base);
            }
            base = multiply(base, base);
        }

        // 先頭の2×2交代ブロックを消去し、残る主小行列をSchur補で置き換える。
        for (std::size_t i = second + 1; i < n; ++i)
        {
            const auto factor_top = multiply(top[i], inverse);
            const auto factor_bottom = multiply(bottom[i], inverse);
            auto &row = matrix[i];
            for (std::size_t j = i + 1; j < n; ++j)
            {
                const auto add = multiply(factor_bottom, top[j]);
                const auto sub = multiply(factor_top, bottom[j]);
                const auto sum = row[j] < Prime - add ? row[j] + add : row[j] - (Prime - add);
                row[j] = sum >= sub ? sum - sub : Prime - (sub - sum);
                matrix[j][i] = negate(row[j]);
            }
        }
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
