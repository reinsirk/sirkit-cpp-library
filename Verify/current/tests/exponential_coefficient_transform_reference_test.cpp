// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Exponential_Coefficient_Transform.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::uint64_t kMod = 998244353;

std::uint64_t power(std::uint64_t base, std::uint64_t exponent)
{
    std::uint64_t result = 1;
    while (exponent != 0)
    {
        if (exponent % 2 == 1)
        {
            result = result * base % kMod;
        }
        base = base * base % kMod;
        exponent /= 2;
    }
    return result;
}

sirkit::FormalPowerSeries naive(const sirkit::FormalPowerSeries &series, std::int64_t step, std::size_t precision)
{
    sirkit::FormalPowerSeries result(precision);
    for (std::size_t n = 0; n < precision; ++n)
    {
        const auto residue = static_cast<std::uint64_t>(step % static_cast<std::int64_t>(kMod) + static_cast<std::int64_t>(kMod)) % kMod;
        const auto base = residue * n % kMod;
        std::uint64_t term = 1;
        std::uint64_t answer = 0;
        for (std::size_t j = 0; j <= n; ++j)
        {
            answer = (answer + term * series.get(n - j)) % kMod;
            term = term * base % kMod * power(j + 1, kMod - 2) % kMod;
        }
        result.set(n, answer);
    }
    return result;
}
} // namespace

int main()
{
    const sirkit::FormalPowerSeries series{1, 2};
    assert(sirkit::exponential_coefficient_transform(series, 1, 3) == sirkit::FormalPowerSeries({1, 3, 6}));
    assert(sirkit::inverse_exponential_coefficient_transform({1, 3, 6}, 1, 3) == sirkit::FormalPowerSeries({1, 2, 0}));
    assert(sirkit::exponential_coefficient_transform(series, 0, 3) == sirkit::FormalPowerSeries({1, 2, 0}));
    assert(sirkit::inverse_exponential_coefficient_transform(series, static_cast<std::int64_t>(kMod), 1) == sirkit::FormalPowerSeries({1}));
    assert(sirkit::exponential_coefficient_transform({}, 4, 1) == sirkit::FormalPowerSeries({0}));
    assert(sirkit::inverse_exponential_coefficient_transform({}, 4, 1) == sirkit::FormalPowerSeries({0}));
    assert(sirkit::exponential_coefficient_transform(series, 3, 0).empty());
    assert(sirkit::inverse_exponential_coefficient_transform(series, -3, 0).empty());
    for (const auto transform : {sirkit::exponential_coefficient_transform, sirkit::inverse_exponential_coefficient_transform})
    {
        bool rejected = false;
        try
        {
            static_cast<void>(transform({}, 0, (std::size_t{1} << 18) + 1));
        }
        catch (const std::length_error &)
        {
            rejected = true;
        }
        assert(rejected);
    }

    std::mt19937_64 rng(20261128);
    for (std::size_t i = 0; i < 120; ++i)
    {
        const std::size_t precision = 1 + rng() % 82;
        const auto step = i == 0 ? std::numeric_limits<std::int64_t>::min()
                                 : static_cast<std::int64_t>(rng() % 5000000000ULL) - 2500000000LL;
        std::vector<std::uint32_t> coeffs(rng() % 80);
        for (auto &coefficient : coeffs)
        {
            coefficient = static_cast<std::uint32_t>(rng() % kMod);
        }
        const sirkit::FormalPowerSeries input(std::move(coeffs));
        const auto expected = naive(input, step, precision);
        assert(sirkit::exponential_coefficient_transform(input, step, precision) == expected);
        assert(sirkit::inverse_exponential_coefficient_transform(expected, step, precision) == input.prefix(precision));
        assert(naive(sirkit::inverse_exponential_coefficient_transform(input, step, precision), step, precision) == input.prefix(precision));
    }
}
