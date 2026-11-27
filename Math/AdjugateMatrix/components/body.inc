#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_MATRIX_ADJUGATE_DEFINED
#define SIRKIT_MATRIX_ADJUGATE_DEFINED
template <class T>
Matrix<T> MatrixAdjugate(const Matrix<T> &matrix)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (matrix.rows() != matrix.cols())
    {
        throw std::invalid_argument("MatrixAdjugate: non-square matrix");
    }
#endif
    const auto n = matrix.rows();
    Matrix<T> augmented(n, ::reinsirk::detail::inverse_augmented_width(n));
    for (std::size_t i = 0; i < n; ++i)
    {
        auto row = augmented.row(i);
        std::copy(matrix.row(i).begin(), matrix.row(i).end(), row.begin());
        row[n + i] = T{1};
    }
    const auto elimination = ::reinsirk::detail::field_eliminate(std::move(augmented), n, true);
    Matrix<T> result(n, n);
    const auto rank = elimination.rank();
    if (rank + 1 < n)
    {
        return result;
    }
    T scale{1};
    for (const auto &pivot : elimination.pivots)
    {
        scale *= pivot;
    }
    if (elimination.row_swaps % 2 != 0)
    {
        scale = T{0} - scale;
    }
    if (rank == n)
    {
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = 0; j < n; ++j)
            {
                result(i, j) = scale * elimination.matrix(i, n + j);
            }
        }
        return result;
    }

    // E A=Rの最終行は0。自由列を1とする右核と、Eの最終行の外積で随伴を復元する。
    std::size_t free_column = 0;
    while (free_column < rank && elimination.pivot_columns[free_column] == free_column)
    {
        ++free_column;
    }
    if ((rank + free_column) % 2 != 0)
    {
        scale = T{0} - scale;
    }
    // 自由列を除く小行列は単位行列。符号込みの1/det(E)が外積を余因子に合わせる。
    const auto left_kernel = elimination.matrix.row(rank).subspan(n);
    const auto fill = [&](std::size_t i, const T &weight)
    {
        const T multiplier = scale * weight;
        for (std::size_t j = 0; j < n; ++j)
        {
            result(i, j) = multiplier * left_kernel[j];
        }
    };
    fill(free_column, T{1});
    for (std::size_t i = 0; i < rank; ++i)
    {
        fill(elimination.pivot_columns[i], T{0} - elimination.matrix(i, free_column));
    }
    return result;
}
#endif
