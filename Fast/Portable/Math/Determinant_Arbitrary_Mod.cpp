// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Determinant_Arbitrary_Mod.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/determinant_arbitrary_mod.md
//
// API一覧
// determinant_mod(matrix,mod): 正方行列の行列式の最小非負剰余を返す。1≤mod≤2^32−1。
// 行列要素は符号付き64ビット。空行列は1 mod mod。合成数の法にも対応。
// n×n行列に対しO(n³ log(mod+1))時間・O(n²)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DETERMINANT_MOD_DEFINED
#define SIRKIT_DETERMINANT_MOD_DEFINED

inline std::uint32_t determinant_mod(std::span<const std::vector<std::int64_t>> matrix, std::uint32_t mod)
{
    if (mod == 0)
    {
        throw std::invalid_argument("determinant_mod: zero modulus");
    }
    const auto n = matrix.size();
    std::vector<std::vector<std::uint32_t>> work;
    work.reserve(n);
    for (const auto &row : matrix)
    {
        auto &values = work.emplace_back();
        values.reserve(n);
        for (auto value : row)
        {
            auto residue = value % static_cast<std::int64_t>(mod);
            if (residue < 0)
            {
                residue += mod;
            }
            values.push_back(static_cast<std::uint32_t>(residue));
        }
    }
    std::uint64_t det = 1 % mod;
    bool negative = false;
    for (std::size_t col = 0; col < n; ++col)
    {
        for (std::size_t row = col + 1; row < n; ++row)
        {
            // 行の引き算と交換で、ピボット列の2要素にEuclidの互除法を行う。
            while (work[row][col] != 0)
            {
                const auto divisor = work[row][col];
                const auto quotient = work[col][col] / divisor;
                work[col][col] %= divisor;
                for (std::size_t j = col + 1; j < n; ++j)
                {
                    const auto subtract = static_cast<std::uint64_t>(quotient) * work[row][j] % mod;
                    const auto value = std::uint64_t{work[col][j]} + mod - subtract;
                    work[col][j] = static_cast<std::uint32_t>(value >= mod ? value - mod : value);
                }
                std::swap(work[col], work[row]);
                negative = !negative;
            }
        }
        det = det * work[col][col] % mod;
        if (det == 0)
        {
            return 0;
        }
    }
    return static_cast<std::uint32_t>(negative ? mod - det : det);
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
