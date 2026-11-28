// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Xor_Convolution_Without_Division.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
template <unsigned Modulus>
struct Residue
{
    unsigned value{};

    Residue &operator+=(Residue other)
    {
        value = (value + other.value) % Modulus;
        return *this;
    }
    Residue &operator-=(Residue other)
    {
        value = (value + Modulus - other.value) % Modulus;
        return *this;
    }
    Residue &operator*=(Residue other)
    {
        value = value * other.value % Modulus;
        return *this;
    }
    bool operator==(const Residue &) const = default;
};

template <unsigned Modulus>
void compare(unsigned dimension, std::mt19937_64 &rng)
{
    const auto n = std::size_t{1} << dimension;
    std::vector<Residue<Modulus>> a(n), b(n), expected(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        a[i].value = static_cast<unsigned>(rng() % Modulus);
        b[i].value = static_cast<unsigned>(rng() % Modulus);
    }
    for (std::size_t i = 0; i < n; ++i)
    {
        for (std::size_t j = 0; j < n; ++j)
        {
            auto &slot = expected[i ^ j].value;
            slot = (slot + a[i].value * b[j].value) % Modulus;
        }
    }
    assert(sirkit::xor_convolution_without_division(a, b) == expected);
    if constexpr (Modulus == 2)
    {
        assert(sirkit::xor_convolution_without_division<true>(a, b) == expected);
    }
}
} // namespace

int main()
{
    using sirkit::xor_convolution_without_division;
    assert(xor_convolution_without_division(std::vector<int>{}, std::vector<int>{}).empty());
    bool rejected = false;
    try
    {
        static_cast<void>(xor_convolution_without_division(std::vector<int>{1}, std::vector<int>{}));
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);
    rejected = false;
    try
    {
        static_cast<void>(xor_convolution_without_division(std::vector<int>(3), std::vector<int>(3)));
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);

    const std::vector<std::uint16_t> a{65535, 32768, 65528, 63};
    const std::vector<std::uint16_t> b{65535, 65000, 61000, 17};
    std::vector<std::uint16_t> expected(a.size());
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        for (std::size_t j = 0; j < b.size(); ++j)
        {
            expected[i ^ j] = static_cast<std::uint16_t>(expected[i ^ j] + static_cast<std::uint32_t>(a[i]) * b[j]);
        }
    }
    assert(xor_convolution_without_division(a, b) == expected);
    assert(xor_convolution_without_division(std::vector<std::int64_t>{-5, 4}, std::vector<std::int64_t>{2, -3}) ==
           std::vector<std::int64_t>({-22, 23}));

    std::mt19937_64 rng(20261128);
    for (unsigned i = 0; i < 80; ++i)
    {
        const auto dimension = static_cast<unsigned>(rng() % 9);
        compare<2>(dimension, rng);
        compare<4>(dimension, rng);
        compare<8>(dimension, rng);
        compare<12>(dimension, rng);
        compare<17>(dimension, rng);
    }
}
