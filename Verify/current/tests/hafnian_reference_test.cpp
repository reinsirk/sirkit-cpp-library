// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Hafnian.cpp"
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
std::int64_t Direct(const std::vector<std::vector<std::int64_t>> &matrix, unsigned unmatched)
{
    if (unmatched == 0)
    {
        return 1;
    }
    const auto first = std::countr_zero(unmatched);
    unmatched ^= 1u << first;
    std::int64_t answer = 0;
    for (unsigned rest = unmatched; rest; rest &= rest - 1)
    {
        const auto next = std::countr_zero(rest);
        answer += matrix[first][next] * Direct(matrix, unmatched ^ (1u << next));
    }
    return answer;
}
}

int main()
{
    using Matrix = std::vector<std::vector<std::int64_t>>;
    std::mt19937 random(22971);
    for (std::size_t n = 0; n <= 12; n += 2)
    {
        for (int attempt = 0; attempt < 40; ++attempt)
        {
            // 対角と下三角は使わないことも確認する。
            Matrix matrix(n, std::vector<std::int64_t>(n, 929));
            for (std::size_t row = 0; row < n; ++row)
            {
                for (std::size_t col = row + 1; col < n; ++col)
                {
                    matrix[row][col] = static_cast<int>(random() % 7) - 3;
                }
            }
            assert(sirkit::hafnian(matrix) == Direct(matrix, (1u << n) - 1));
        }
    }
    assert(sirkit::hafnian(Matrix(3, std::vector<std::int64_t>(3, 1))) == 0);
    try
    {
        static_cast<void>(sirkit::hafnian(Matrix(41, std::vector<std::int64_t>(41))));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
#ifndef SKIP_RUNTIME_CHECKS
    try
    {
        static_cast<void>(sirkit::hafnian(Matrix{{1, 2}, {3}}));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
