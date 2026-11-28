// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Polynomial_Moments.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;

std::uint32_t Reduce(std::int64_t value)
{
    const auto remainder = value % kMod;
    return static_cast<std::uint32_t>((remainder + kMod) % kMod);
}

std::uint32_t SlowPower(std::uint32_t value, std::size_t exponent)
{
    std::uint64_t result = 1;
    for (std::size_t i = 0; i < exponent; ++i)
    {
        result = result * value % kMod;
    }
    return static_cast<std::uint32_t>(result);
}

void CheckWeighted(const std::vector<std::int64_t> &points, const std::vector<std::int64_t> &weights, std::size_t count)
{
    const auto actual = sirkit::weighted_power_sums(points, weights, count);
    assert(actual.size() == count);
    for (std::size_t degree = 0; degree < count; ++degree)
    {
        std::uint64_t expected = 0;
        for (std::size_t i = 0; i < points.size(); ++i)
        {
            expected = (expected + std::uint64_t{Reduce(weights[i])} * SlowPower(Reduce(points[i]), degree)) % kMod;
        }
        assert(actual.get(degree) == expected);
    }
}

std::uint32_t Inverse(std::uint32_t value)
{
    std::uint64_t result = 1;
    std::uint64_t base = value;
    for (auto exponent = kMod - 2; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1u) != 0)
        {
            result = result * base % kMod;
        }
        base = base * base % kMod;
    }
    return static_cast<std::uint32_t>(result);
}

void CheckComposition(const std::vector<std::int64_t> &poly, std::int64_t scale, std::size_t count,
                      std::int64_t shift)
{
    const auto actual = sirkit::exponential_composition(sirkit::FormalPowerSeries(poly), scale, count, shift);
    assert(actual.size() == count);
    std::vector<std::uint64_t> series(count), term(count), expected(count);
    series[0] = (1 + Reduce(shift)) % kMod;
    std::uint64_t inverse_factorial = 1;
    for (std::size_t i = 1; i < count; ++i)
    {
        inverse_factorial = inverse_factorial * Inverse(static_cast<std::uint32_t>(i)) % kMod;
        series[i] = inverse_factorial * SlowPower(Reduce(scale), i) % kMod;
    }

    // Horner法と素朴な畳み込みで合成そのものを参照計算。
    for (auto i = poly.size(); i != 0; --i)
    {
        for (std::size_t degree = 0; degree < count; ++degree)
        {
            std::uint64_t value = 0;
            for (std::size_t j = 0; j <= degree; ++j)
            {
                value = (value + term[j] * series[degree - j]) % kMod;
            }
            expected[degree] = value;
        }
        expected[0] = (expected[0] + Reduce(poly[i - 1])) % kMod;
        term.swap(expected);
    }
    for (std::size_t i = 0; i < count; ++i)
    {
        assert(actual.get(i) == term[i]);
    }
}

template <class Error, class Call>
void Reject(Call call)
{
    bool rejected = false;
    try
    {
        call();
    }
    catch (const Error &)
    {
        rejected = true;
    }
    assert(rejected);
}
} // namespace

int main()
{
    CheckWeighted({}, {}, 0);
    CheckWeighted({}, {}, 8);
    CheckWeighted({0, 0, 1, -2}, {-1, 10, 0, 1}, 37);
    CheckWeighted({std::numeric_limits<std::int64_t>::min(), -2, 0, 3},
                  {-3, std::numeric_limits<std::int64_t>::max(), 11, -1}, 14);
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const auto unsigned_result = sirkit::weighted_power_sums(std::vector{kMax}, std::vector{kMax}, 2);
    assert(unsigned_result.get(0) == kMax % kMod);
    assert(unsigned_result.get(1) == (kMax % kMod) * (kMax % kMod) % kMod);
    assert(sirkit::exponential_composition(sirkit::FormalPowerSeries{5}, 1, 0).empty());

    CheckComposition({}, 2, 8, 1);
    CheckComposition({1, 2, -3}, -static_cast<std::int64_t>(kMod), 8, -2);
    CheckComposition({1, -3, 5}, -7, 23, 0);
    CheckComposition({1, -3, 5}, 7, 1, -1);
    std::mt19937_64 random(1875);
    for (int trial = 0; trial < 50; ++trial)
    {
        const auto length = 10 + random() % 86;
        const auto count = 2 + random() % 69;
        std::vector<std::int64_t> points(length), weights(length);
        for (std::size_t i = 0; i < length; ++i)
        {
            points[i] = static_cast<std::int64_t>(random() % 31) - 15;
            weights[i] = static_cast<std::int64_t>(random() % 71) - 35;
        }
        CheckWeighted(points, weights, count);
        CheckComposition(weights, static_cast<std::int64_t>(random() % 7),
                         static_cast<std::size_t>(trial % 23 + 1), trial % 9 - 4);
    }
    Reject<std::invalid_argument>([]
                                  {
                                      (void)sirkit::weighted_power_sums(std::vector{1}, std::vector<int>{}, 0);
                                  });
    Reject<std::length_error>([]
                              {
                                  (void)sirkit::weighted_power_sums(std::vector{1}, std::vector{1}, sirkit::FormalPowerSeries::max_precision() + 1);
                              });
    Reject<std::length_error>([]
                              {
                                  (void)sirkit::exponential_composition(sirkit::FormalPowerSeries{1}, 1, sirkit::FormalPowerSeries::max_precision() + 1);
                              });
}
