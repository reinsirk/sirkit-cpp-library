// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Determinant_Arbitrary_Mod.cpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

int main()
{
    std::mt19937 random(19328);
    const std::array<std::uint32_t, 8> moduli{1, 2, 4, 17, 1000, 65536, 998244353, UINT32_MAX};
    for (std::size_t n = 0; n <= 6; ++n)
    {
        for (int trial = 0; trial < 90; ++trial)
        {
            std::vector<std::vector<std::int64_t>> matrix(n, std::vector<std::int64_t>(n));
            for (auto &row : matrix)
            {
                for (auto &value : row)
                {
                    value = static_cast<std::int64_t>(random() % 15) - 7;
                }
            }
            std::int64_t exact = 0;
            std::vector<std::size_t> order(n);
            std::iota(order.begin(), order.end(), std::size_t{0});
            do
            {
                std::int64_t product = 1;
                for (std::size_t i = 0; i < n; ++i)
                {
                    product *= matrix[i][order[i]];
                    for (std::size_t j = 0; j < i; ++j)
                    {
                        if (order[j] > order[i])
                        {
                            product = -product;
                        }
                    }
                }
                exact += product;
            } while (std::ranges::next_permutation(order).found);
            for (auto modulus : moduli)
            {
                const auto expected = (exact % modulus + modulus) % modulus;
                assert(sirkit::determinant_mod(matrix, modulus) == expected);
            }
        }
    }
    const std::vector<std::vector<std::int64_t>> large{{std::numeric_limits<std::int64_t>::min()}};
    assert(sirkit::determinant_mod(large, 5) == 2);
    try
    {
        sirkit::determinant_mod({}, 0);
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#ifndef SKIP_RUNTIME_CHECKS
    try
    {
        const std::vector<std::vector<std::int64_t>> nonsquare{{0, 1}};
        sirkit::determinant_mod(nonsquare, 2);
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
