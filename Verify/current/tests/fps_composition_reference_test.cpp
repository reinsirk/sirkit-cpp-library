// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/FPS_Composition.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Polynomial = sirkit::FormalPowerSeries;
constexpr auto kMod = Polynomial::modulus();

Polynomial Naive(const Polynomial &outer, const Polynomial &inner, std::size_t precision)
{
    std::vector<std::uint32_t> result(precision);
    for (std::size_t j = outer.size(); j != 0; --j)
    {
        std::vector<std::uint32_t> next(precision);
        next[0] = outer[j - 1];
        for (std::size_t i = 0; i < precision; ++i)
        {
            for (std::size_t k = 0; k < inner.size() && i + k < precision; ++k)
            {
                next[i + k] = static_cast<std::uint32_t>((next[i + k] + std::uint64_t{result[i]} * inner[k]) % kMod);
            }
        }
        result.swap(next);
    }
    return Polynomial(std::move(result));
}

void CheckInverse(const Polynomial &value, std::size_t precision)
{
    const auto inverse = sirkit::fps_compositional_inverse(value, precision);
    const Polynomial x = Polynomial{0, 1}.prefix(precision);
    assert(inverse.size() == precision);
    assert(Naive(value, inverse, precision) == x);
    assert(Naive(inverse, value, precision) == x);
}

void CheckProjection(const Polynomial &value, const Polynomial &weight, std::size_t degree, std::size_t count)
{
    auto current = weight.prefix(degree + 1).coefficients();
    const auto result = sirkit::fps_power_projection(value, weight, degree, count);
    assert(result.size() == count);
    for (std::size_t e = 0; e < count; ++e)
    {
        assert(result[e] == current[degree]);
        std::vector<std::uint32_t> next(degree + 1);
        for (std::size_t i = 0; i <= degree; ++i)
        {
            for (std::size_t j = 0; j < value.size() && i + j <= degree; ++j)
            {
                next[i + j] = static_cast<std::uint32_t>((next[i + j] + std::uint64_t{current[i]} * value[j]) % kMod);
            }
        }
        current.swap(next);
    }
}
}

int main()
{
    using sirkit::fps_compose;
    using sirkit::fps_compositional_inverse;
    using sirkit::fps_power_projection;
    assert(fps_compose({1}, {1}, 0).empty());
    assert(fps_compositional_inverse({1}, 0).empty());
    assert(fps_power_projection({1}, {1}, 0, 0).empty());
    assert((fps_power_projection({2, 1}, {1, 2, 1}, 2, 4) == Polynomial{1, 4, 13, 38}));
    assert(fps_power_projection({2, 1}, {1, 2, 1}, 2, 1) == Polynomial{1});
    CheckProjection({3}, {4}, 0, 8);
    assert(fps_compose({1, 2, 3}, {0, 0, 5}, 1) == Polynomial{1});
    assert((fps_compose({1, 2, 3}, {0, 4, 5}, 4) == Polynomial{1, 8, 58, 120}));
    assert((fps_compose({}, {0, 3}, 4) == Polynomial{0, 0, 0, 0}));
    assert((fps_compositional_inverse({0, 4}, 1) == Polynomial{0}));
    for (const auto &invalid : {Polynomial{1, 1}, Polynomial{0, 0, 1}})
    {
        try
        {
            static_cast<void>(fps_compositional_inverse(invalid, 4));
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
    try
    {
        static_cast<void>(fps_compose({1}, {1}, 4));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    try
    {
        static_cast<void>(fps_compose({1}, {0, 1}, (std::size_t{1} << 18) + 1));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    for (const std::size_t oversized : {std::size_t{1} << 18, (std::size_t{1} << 18) + 1})
    {
        try
        {
            static_cast<void>(fps_power_projection({0, 1}, {1}, oversized, 1));
            assert(false);
        }
        catch (const std::length_error &)
        {
        }
    }
    try
    {
        static_cast<void>(fps_power_projection({0, 1}, {1}, 2, (std::size_t{1} << 18) + 1));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    try
    {
        static_cast<void>(fps_compositional_inverse({0, 1}, (std::size_t{1} << 18) + 1));
        assert(false);
    }
    catch (const std::length_error &)
    {
    }

    std::mt19937 random(59437);
    for (const std::size_t precision : {2, 3, 4, 5, 7, 15, 16, 31, 47, 64, 65, 89})
    {
        for (std::size_t trial = 0; trial < 4; ++trial)
        {
            Polynomial outer(precision + trial);
            Polynomial inner(precision > trial ? precision - trial : 0);
            for (std::size_t i = 0; i < outer.size(); ++i)
            {
                outer.set(i, random());
            }
            for (std::size_t i = 1 + trial; i < inner.size(); ++i)
            {
                inner.set(i, random());
            }
            assert(fps_compose(outer, inner, precision) == Naive(outer, inner, precision));
            CheckProjection(inner, outer, precision - 1, precision + 3);
            inner.resize(std::max(inner.size(), std::size_t{1}));
            inner.set(0, random());
            CheckProjection(inner, outer, precision - 1, precision + 1);
        }
        Polynomial value(precision);
        value.set(1, 1 + random() % (kMod - 1));
        for (std::size_t i = 2; i < precision; ++i)
        {
            value.set(i, random());
        }
        CheckInverse(value, precision);
    }

    constexpr std::size_t kLarge = 4093;
    const Polynomial geometric(std::vector<std::uint32_t>(kLarge, 1));
    const auto fibonacci = fps_compose(geometric, {0, 1, 1}, kLarge);
    assert(fibonacci[0] == 1 && fibonacci[1] == 1);
    for (std::size_t i = 2; i < kLarge; ++i)
    {
        assert(fibonacci[i] == (fibonacci[i - 1] + fibonacci[i - 2]) % kMod);
    }
    auto geometric_times_x = geometric;
    geometric_times_x.set(0, 0);
    const auto inverse = fps_compositional_inverse(geometric_times_x, kLarge);
    assert(inverse[0] == 0);
    for (std::size_t i = 1; i < kLarge; ++i)
    {
        assert(inverse[i] == (i % 2 == 1 ? 1 : kMod - 1));
    }
}
