// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Gaussian_Elimination.cpp"
#include "../../../Math/Adjugate_Matrix.cpp"
#include "reference_modint.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <vector>

namespace
{
template <class Mint>
Mint cofactor(const sirkit::Matrix<Mint> &matrix, std::size_t row, std::size_t column)
{
    const auto n = matrix.rows();
    std::vector<std::size_t> columns, rows;
    for (std::size_t i = 0; i < n; ++i)
    {
        if (i != row)
        {
            rows.push_back(i);
        }
        if (i != column)
        {
            columns.push_back(i);
        }
    }
    Mint determinant{0};
    do
    {
        bool odd = ((row + column) % 2 != 0);
        Mint product{1};
        for (std::size_t i = 0; i < columns.size(); ++i)
        {
            product *= matrix(rows[i], columns[i]);
            for (std::size_t j = 0; j < i; ++j)
            {
                odd ^= columns[j] > columns[i];
            }
        }
        determinant += odd ? Mint{0} - product : product;
    } while (std::next_permutation(columns.begin(), columns.end()));
    return determinant;
}

template <class Mint>
void compare(const sirkit::Matrix<Mint> &matrix)
{
    const auto answer = sirkit::MatrixAdjugate(matrix);
    const auto n = matrix.rows();
    assert(answer.rows() == n && answer.cols() == n);
    for (std::size_t i = 0; i < n; ++i)
    {
        for (std::size_t j = 0; j < n; ++j)
        {
            const auto expected = cofactor(matrix, j, i);
            assert(answer(i, j) == expected);
        }
    }
}

template <class Mint>
void cases()
{
    compare(sirkit::Matrix<Mint>(0, 0));
    compare(sirkit::Matrix<Mint>{{0}});
    compare(sirkit::Matrix<Mint>{{0, 1}, {0, 0}});
    compare(sirkit::Matrix<Mint>{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}});
    compare(sirkit::Matrix<Mint>{{0, 0, 1}, {1, 0, 0}, {0, 0, 0}});
    compare(sirkit::Matrix<Mint>{{0, 1, 2}, {1, 3, 5}, {3, 7, 9}});
    std::mt19937 random(948572);
    for (std::size_t n = 1; n <= 6; ++n)
    {
        for (unsigned trial = 0; trial < 130; ++trial)
        {
            sirkit::Matrix<Mint> a(n, n);
            const auto each_size = trial % 4 == 0 ? std::size_t{1} : n;
            for (std::size_t i = 0; i < n; ++i)
            {
                for (std::size_t j = 0; j < each_size; ++j)
                {
                    a(i, j) = Mint(static_cast<int>(random() % 11) - 5);
                }
            }
            compare(a);
        }
    }
}
}

int main()
{
    cases<sirkit_verify::ReferenceModInt<998244353>>();
    cases<sirkit_verify::ReferenceModInt<2>>();
#ifndef SKIP_RUNTIME_CHECKS
    try
    {
        sirkit::MatrixAdjugate(sirkit::Matrix<sirkit_verify::ReferenceModInt<998244353>>(2, 1));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
