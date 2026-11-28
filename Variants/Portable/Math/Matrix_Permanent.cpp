// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Math/Matrix_Permanent.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/matrix_permanent.md
// 参考（Nijenhuis と Wilf, Combinatorial Algorithms 第23章）: https://shop.elsevier.com/books/combinatorial-algorithms/nijenhuis/978-0-12-519260-6
//
// API一覧
// matrix_permanent<T>(n,value_at,one=T{1}): n×n行列のパーマネントを求める。空行列はone。
// value_at(row,col)は行列の要素。T{}は加法単位元で、oneは乗法単位元。剰余の法が合成数でも使える。
// 環演算がO(1)ならO(n 2^n)時間・O(n^2)領域。n>=64はlength_error。
// 符号付き組み込み整数型では、途中の和・積も型に収まること。uint64_tで法2^64を計算できる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MATRIX_PERMANENT_DEFINED
#define SIRKIT_MATRIX_PERMANENT_DEFINED

template <class T, class ValueAt>
T matrix_permanent(std::size_t n, const ValueAt &value_at, const T &one = T{1})
{
    if (n >= std::numeric_limits<std::uint64_t>::digits)
    {
        throw std::length_error("matrix_permanent: size exceeds the subset mask");
    }
    if (n == 0)
    {
        return one;
    }
    std::vector<T> columns(n * n);
    for (std::size_t column = 0; column < n; ++column)
    {
        for (std::size_t row = 0; row < n; ++row)
        {
            columns[column * n + row] = static_cast<T>(value_at(row, column));
        }
    }
    std::vector<T> row_sums(n);
    T answer{};
    const auto limit = std::uint64_t{1} << n;
    for (std::uint64_t step = 1; step < limit; ++step)
    {
        // グレイ符号で変わるビットはstepの最下位の1。1回で1列だけ加減する。
        const auto column = static_cast<std::size_t>(std::countr_zero(step));
        const auto offset = column * n;
        T product = one;
        if ((step >> (column + 1) & 1) == 0)
        {
            for (std::size_t row = 0; row < n; ++row)
            {
                row_sums[row] += columns[offset + row];
                product *= row_sums[row];
            }
        }
        else
        {
            for (std::size_t row = 0; row < n; ++row)
            {
                row_sums[row] -= columns[offset + row];
                product *= row_sums[row];
            }
        }
        // 部分集合サイズの偶奇は毎回反転するので、stepの偶奇に等しい。
        if (((step ^ n) & 1) == 0)
        {
            answer += product;
        }
        else
        {
            answer -= product;
        }
    }
    return answer;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
