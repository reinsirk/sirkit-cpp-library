// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_WALSH_HADAMARD_DEFINED
#include "../../../Math/Walsh_Hadamard.cpp"
#endif
#include <algorithm>
#include <array>
#include "reference_modint.hpp"
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
// 逆変換が素数法を前提としないことを確認する。
struct Mod9
{
    template <std::integral I>
    constexpr Mod9(I input) : value(static_cast<unsigned>(input % 9))
    {
    }

    constexpr Mod9() = default;

    friend constexpr bool operator==(Mod9, Mod9) = default;
    friend constexpr Mod9 operator+(Mod9 a, Mod9 b)
    {
        return Mod9(a.value + b.value);
    }
    friend constexpr Mod9 operator-(Mod9 a, Mod9 b)
    {
        return Mod9(a.value + 9 - b.value);
    }
    friend constexpr Mod9 operator*(Mod9 a, Mod9 b)
    {
        return Mod9(a.value * b.value);
    }
    friend constexpr Mod9 operator/(Mod9 a, Mod9 b)
    {
        for (unsigned i = 1; i < 9; ++i)
        {
            if (i * b.value % 9 == 1)
            {
                return a * Mod9(i);
            }
        }
        std::abort();
    }

    constexpr Mod9 &operator+=(Mod9 other)
    {
        return *this = *this + other;
    }
    constexpr Mod9 &operator*=(Mod9 other)
    {
        return *this = *this * other;
    }

    unsigned value = 0;
};

template <class T>
std::vector<T> xor_convolution_reference(const std::vector<T> &left, const std::vector<T> &right)
{
    std::vector<T> result(left.size());
    for (std::size_t i = 0; i < left.size(); ++i)
    {
        for (std::size_t j = 0; j < right.size(); ++j)
        {
            result[i ^ j] += left[i] * right[j];
        }
    }
    return result;
}

#ifndef SKIP_RUNTIME_CHECKS
template <class Operation>
void expect_invalid(Operation operation)
{
    try
    {
        operation();
    }
    catch (const std::invalid_argument &)
    {
        return;
    }
    assert(false);
}
#endif
} // namespace

int main(int argc, char **)
{
    using sirkit::bitwise_xor_convolution;
    using sirkit::bitwise_xor_square;
    using sirkit::inverse_walsh_hadamard_transform;
    using sirkit::walsh_hadamard_transform;
    using Integers = std::vector<std::int64_t>;

    Integers empty;
    walsh_hadamard_transform(std::span(empty));
    inverse_walsh_hadamard_transform(std::span(empty));
    assert(empty.empty());
    assert(bitwise_xor_convolution(empty, empty).empty());
    assert(bitwise_xor_square(empty).empty());
    assert(bitwise_xor_convolution(Integers{-7}, Integers{5}) == Integers{-35});
    assert(bitwise_xor_square(Integers{-7}) == Integers{49});

#ifndef SKIP_RUNTIME_CHECKS
    std::array<int, 3> invalid{};
    expect_invalid([&]
                   {
                       walsh_hadamard_transform(std::span(invalid));
                   });
    expect_invalid([&]
                   {
                       inverse_walsh_hadamard_transform(std::span(invalid));
                   });
    expect_invalid([&]
                   {
                       bitwise_xor_convolution(Integers{1}, Integers{1, 2});
                   });
    expect_invalid([&]
                   {
                       bitwise_xor_convolution(Integers{1, 2, 3}, Integers{3, 2, 1});
                   });
    expect_invalid([&]
                   {
                       bitwise_xor_square(Integers{3, 2, 1});
                   });
#endif

    std::mt19937_64 rng(291187);
    for (unsigned iteration = 0; iteration < 120; ++iteration)
    {
        const std::size_t length = std::size_t{1} << (iteration % 9);
        Integers a(length), b(length);
        for (std::size_t j = 0; j < length; ++j)
        {
            a[j] = static_cast<std::int64_t>(rng() % 20001) - 10000;
            b[j] = static_cast<std::int64_t>(rng() % 20001) - 10000;
        }
        assert(bitwise_xor_convolution(a, b) == xor_convolution_reference(a, b));
        assert(bitwise_xor_square(a) == xor_convolution_reference(a, a));

        auto actual = a;
        walsh_hadamard_transform(std::span(actual));
        for (std::size_t k = 0; k < length; ++k)
        {
            std::int64_t expected = 0;
            for (std::size_t j = 0; j < length; ++j)
            {
                expected += std::popcount(j & k) % 2 == 0 ? a[j] : -a[j];
            }
            assert(actual[k] == expected);
        }
        inverse_walsh_hadamard_transform(std::span(actual));
        assert(actual == a);
    }

    std::vector<Mod9> field_like_a{1, 2, 5, 3, 8, 8, 1, 3}, field_like_b{2, 1, 4, 1, 4, 2, 5, 8};
    assert(bitwise_xor_convolution(field_like_a, field_like_b) == xor_convolution_reference(field_like_a, field_like_b));
    assert(bitwise_xor_square(field_like_a) == xor_convolution_reference(field_like_a, field_like_a));
    const auto original = field_like_a;
    walsh_hadamard_transform(std::span(field_like_a));
    inverse_walsh_hadamard_transform(std::span(field_like_a));
    assert(field_like_a == original);

    if (argc > 1)
    {
        using Mint = sirkit_verify::ReferenceModInt<998244353>;
        constexpr std::size_t kLength = std::size_t{1} << 20, kImpulse = 712345;
        std::vector<Mint> data(kLength), impulse(kLength);
        for (auto &value : data)
        {
            value = Mint(rng() % 998244353);
        }
        impulse[kImpulse] = 999;
        const auto result = bitwise_xor_convolution(data, std::move(impulse));
        for (std::size_t k = 0; k < kLength; ++k)
        {
            assert(result[k] == data[k ^ kImpulse] * 999);
        }
    }
}
