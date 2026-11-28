// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Bivariate_Formal_Power_Series.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
constexpr std::uint32_t kModulus = 998244353;

std::uint32_t Pow(std::uint64_t base, std::uint32_t exponent)
{
    std::uint64_t result = 1;
    while (exponent != 0)
    {
        if (exponent % 2 != 0)
        {
            result = result * base % kModulus;
        }
        exponent /= 2;
        base = base * base % kModulus;
    }
    return static_cast<std::uint32_t>(result);
}

std::vector<std::uint32_t> Direct(const std::vector<std::uint32_t> &a, std::size_t rows, std::size_t cols)
{
    std::vector<std::uint32_t> answer(rows * cols);
    const auto inv = Pow(a[0], kModulus - 2);
    answer[0] = inv;
    for (std::size_t x = 0; x < rows; ++x)
    {
        for (std::size_t y = 0; y < cols; ++y)
        {
            if (x == 0 && y == 0)
            {
                continue;
            }
            std::uint64_t total = 0;
            for (std::size_t i = 0; i <= x; ++i)
            {
                for (std::size_t j = 0; j <= y; ++j)
                {
                    if (i != 0 || j != 0)
                    {
                        total = (total + static_cast<std::uint64_t>(a[i * cols + j]) * answer[(x - i) * cols + y - j]) % kModulus;
                    }
                }
            }
            answer[x * cols + y] = static_cast<std::uint32_t>((kModulus - total) * inv % kModulus);
        }
    }
    return answer;
}
}

int main()
{
    std::mt19937 random(917231);
    for (const auto &[rows, cols] : {std::pair{1u, 1u}, std::pair{1u, 7u}, std::pair{2u, 3u}, std::pair{3u, 2u},
                                     std::pair{3u, 18u}, std::pair{18u, 3u}, std::pair{30u, 1u}, std::pair{19u, 23u},
                                     std::pair{33u, 30u}, std::pair{6u, 31u}})
    {
        for (int iteration = 0; iteration < 4; ++iteration)
        {
            std::vector<std::uint32_t> a(rows * cols);
            for (auto &entry : a)
            {
                entry = random() % kModulus;
            }
            a[0] = 1 + random() % (kModulus - 1);
            assert(sirkit::bivariate_fps_inverse(a, rows, cols) == Direct(a, rows, cols));
        }
    }
    const std::size_t rows = 201, columns = 281;
    std::vector<std::uint32_t> coefficients(rows * columns);
    coefficients[0] = 1;
    coefficients[1] = kModulus - 1;
    coefficients[columns] = kModulus - 1;
    const auto series = sirkit::bivariate_fps_inverse(coefficients, rows, columns);
    std::vector<std::uint32_t> row(columns, 1);
    for (std::size_t x = 0; x < rows; ++x)
    {
        for (std::size_t y = 1; y < columns; ++y)
        {
            if (x != 0)
            {
                row[y] = (row[y] + row[y - 1]) % kModulus;
            }
            assert(series[x * columns + y] == row[y]);
        }
    }
    assert(sirkit::bivariate_fps_inverse({}, 0, 7).empty());
    assert(sirkit::bivariate_fps_inverse({}, 7, 0).empty());
    assert(sirkit::bivariate_fps_inverse({}, 0, 0).empty());

    try
    {
        static_cast<void>(sirkit::bivariate_fps_inverse({}, 8192, 8192));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    try
    {
        static_cast<void>(sirkit::bivariate_fps_inverse(std::vector<std::uint32_t>{0}, 1, 1));
        assert(false);
    }
    catch (const std::domain_error &)
    {
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &bad : {std::vector<std::uint32_t>{1}, {kModulus, 0}, {1, kModulus}})
    {
        try
        {
            static_cast<void>(sirkit::bivariate_fps_inverse(bad, 2, 1));
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
#endif
}
