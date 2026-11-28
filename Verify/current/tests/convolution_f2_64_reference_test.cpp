// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Convolution_F2_64.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

static std::uint64_t slow_multiply(std::uint64_t a, std::uint64_t b)
{
    std::uint64_t result = 0;
    for (int i = 0; i < 64; ++i)
    {
        if ((b & 1) != 0)
        {
            result ^= a;
        }
        const auto leading = a >> 63;
        a = (a << 1) ^ (27 * leading);
        b >>= 1;
    }
    return result;
}

static void check(std::vector<std::uint64_t> a, std::vector<std::uint64_t> b)
{
    std::vector<std::uint64_t> expected(a.size() + b.size() - 1);
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        for (std::size_t j = 0; j < b.size(); ++j)
        {
            expected[i + j] ^= slow_multiply(a[i], b[j]);
        }
    }
    assert(sirkit::convolution_f2_64(a, b) == expected);
}

static void check_length(std::span<const std::uint64_t> a, std::span<const std::uint64_t> b)
{
    bool caught = false;
    try
    {
        (void)sirkit::convolution_f2_64(a, b);
    }
    catch (const std::length_error &)
    {
        caught = true;
    }
    assert(caught);
}

int main()
{
    const std::array<std::uint64_t, 1> unit{1};
    assert(sirkit::convolution_f2_64({}, unit).empty());
    assert(sirkit::convolution_f2_64(unit, {}).empty());
    check({1, 2, 4}, {8, 16});
    check({0, 0}, {0});
    assert(sirkit::multiply_f2_64(std::uint64_t{1} << 63, 2) == 27);
    std::mt19937_64 rng(20261128);
    for (int i = 0; i < 2000; ++i)
    {
        const auto a = rng(), b = rng();
        assert(sirkit::multiply_f2_64(a, b) == slow_multiply(a, b));
    }
    for (int t = 0; t < 32; ++t)
    {
        std::vector<std::uint64_t> a(rng() % 65 + 1), b(rng() % 55 + 1);
        for (auto &x : a)
        {
            x = rng();
        }
        for (auto &x : b)
        {
            x = rng();
        }
        check(a, b);
    }
    constexpr std::size_t kSize = 2049;
    constexpr auto kValue = std::uint64_t{0xfedcba9876543210ULL};
    const auto repeated = std::vector<std::uint64_t>(kSize, kValue);
    const auto square = sirkit::convolution_f2_64(repeated, repeated);
    const auto product = slow_multiply(kValue, kValue);
    assert(square.size() == 2 * kSize - 1);
    for (std::size_t i = 0; i < square.size(); ++i)
    {
        assert(square[i] == ((i % 2 == 0) ? product : 0));
    }
    const auto too_long = std::vector<std::uint64_t>((std::size_t{1} << 21) + 1);
    check_length(too_long, unit);
    check_length(std::span(too_long).first(too_long.size() - 1), std::span(too_long).first(2));
}
