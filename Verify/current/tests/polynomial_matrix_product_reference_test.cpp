// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Polynomial_Matrix_Product.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::int64_t kMod = 998244353;
using Input = std::vector<std::vector<std::vector<std::int64_t>>>;
using Output = std::vector<std::vector<std::uint32_t>>;

std::uint64_t Normalize(std::int64_t value)
{
    return static_cast<std::uint64_t>((value % kMod + kMod) % kMod);
}

Output Scan(const Input &matrix, std::int64_t first, std::size_t count)
{
    const auto n = matrix.size();
    Output prefix(n, std::vector<std::uint32_t>(n));
    for (std::size_t i = 0; i < n; ++i)
    {
        prefix[i][i] = 1;
    }
    for (std::size_t t = 0; t < count; ++t)
    {
        const auto x = (Normalize(first) + t) % kMod;
        Output values(n, std::vector<std::uint32_t>(n)), next(values);
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = 0; j < n; ++j)
            {
                std::uint64_t power = 1;
                for (const auto coefficient : matrix[i][j])
                {
                    values[i][j] = static_cast<std::uint32_t>((values[i][j] + Normalize(coefficient) * power) % kMod);
                    power = power * x % kMod;
                }
            }
        }
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = 0; j < n; ++j)
            {
                for (std::size_t k = 0; k < n; ++k)
                {
                    next[i][j] = static_cast<std::uint32_t>((next[i][j] + std::uint64_t{prefix[i][k]} * values[k][j]) % kMod);
                }
            }
        }
        prefix.swap(next);
    }
    return prefix;
}

void Verify(const Input &matrix, std::int64_t first, std::size_t count)
{
    assert(sirkit::polynomial_matrix_interval_product(matrix, first, count) == Scan(matrix, first, count));
}
} // namespace

int main()
{
    using sirkit::polynomial_matrix_interval_product;
    const Input shear{{{1}, {0, 1}}, {{}, {1}}};
    Verify({}, -1, 20);
    Verify(shear, -3, 0);
    Verify(shear, 2, 3);
    Verify(shear, kMod - 3, 503);
    Verify({{{}, {kMod}}, {{-kMod}, {0, 1}}}, -7, 12);

    const Input constant{{{1}, {1, kMod, 0}}, {{}, {1}}};
    constexpr auto kHuge = std::numeric_limits<std::uint64_t>::max();
    assert(polynomial_matrix_interval_product(constant, 1, kHuge) == Output({{1, static_cast<std::uint32_t>(kHuge % kMod)}, {0, 1}}));
    assert(polynomial_matrix_interval_product(shear, -5, std::uint64_t{2} * kMod + 3) == Output({{1, kMod - 12}, {0, 1}}));
    assert(polynomial_matrix_interval_product(shear, -5, kMod) == Output({{1, 0}, {0, 1}}));

    const Input order_sensitive{{{1, 1}, {0, 1}}, {{1}, {1, -1}}};
    Verify(order_sensitive, std::numeric_limits<std::int64_t>::min(), 500);
    Verify(order_sensitive, std::numeric_limits<std::int64_t>::max(), 129);
    const Input at_maximum{{std::vector<std::int64_t>(257, 1)}};
    Verify(at_maximum, 9, 31);

    std::mt19937 random(261433);
    for (int trial = 0; trial < 265; ++trial)
    {
        const auto n = random() % 4 + 1;
        Input matrix(n, std::vector<std::vector<std::int64_t>>(n));
        for (auto &row : matrix)
        {
            for (auto &poly : row)
            {
                poly.resize(random() % 6);
                for (auto &value : poly)
                {
                    value = static_cast<std::int64_t>(random() % 201) - 100;
                }
            }
        }
        Verify(matrix, static_cast<std::int64_t>(random() % 1601) - 800, random() % 170);
    }

    bool invalid_shape = false;
    try
    {
        static_cast<void>(polynomial_matrix_interval_product(Input{{{1}, {0}}}, 0, 0));
    }
    catch (const std::invalid_argument &)
    {
        invalid_shape = true;
    }
    assert(invalid_shape);

    bool excessive_degree = false;
    try
    {
        const Input high{{std::vector<std::int64_t>(258, 1)}};
        static_cast<void>(polynomial_matrix_interval_product(high, 0, 0));
    }
    catch (const std::length_error &)
    {
        excessive_degree = true;
    }
    assert(excessive_degree);
}
