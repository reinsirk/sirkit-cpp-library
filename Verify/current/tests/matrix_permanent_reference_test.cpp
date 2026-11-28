// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Matrix_Permanent.cpp"
#include "reference_modint.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

template <class T, class ValueAt>
static T brute(std::size_t size, const ValueAt &value_at)
{
    std::vector<std::size_t> permutation(size);
    std::iota(permutation.begin(), permutation.end(), std::size_t{0});
    T total{};
    do
    {
        T product{1};
        for (std::size_t i = 0; i < size; ++i)
        {
            product *= static_cast<T>(value_at(i, permutation[i]));
        }
        total += product;
    } while (std::ranges::next_permutation(permutation).found);
    return total;
}

int main()
{
    const auto fail_if_called = [](std::size_t, std::size_t)
    {
        assert(false);
        return 0;
    };
    assert(sirkit::matrix_permanent<int>(0, fail_if_called) == 1);
    bool caught = false;
    try
    {
        (void)sirkit::matrix_permanent<int>(64, fail_if_called);
    }
    catch (const std::length_error &)
    {
        caught = true;
    }
    assert(caught);

    std::mt19937 rng(3479572);
    for (int trial = 0; trial < 220; ++trial)
    {
        const std::size_t size = 1 + rng() % 7;
        std::vector<int> entries(size * size);
        for (auto &entry : entries)
        {
            entry = static_cast<int>(rng() % 19) - 9;
        }
        const auto value_at = [&](std::size_t row, std::size_t column)
        {
            return entries[row * size + column];
        };
        using Int = std::int64_t;
        using Mod = sirkit_verify::ReferenceModInt<12>;
        const auto expected = brute<Int>(size, value_at);
        assert(sirkit::matrix_permanent<Int>(size, value_at) == expected);
        assert(sirkit::matrix_permanent<Mod>(size, value_at) == Mod{expected});
    }

    const auto high = [](std::size_t row, std::size_t column)
    {
        return std::numeric_limits<std::uint64_t>::max() - (row * 9811 + column * 103);
    };
    assert(sirkit::matrix_permanent<std::uint64_t>(5, high) == brute<std::uint64_t>(5, high));
}
