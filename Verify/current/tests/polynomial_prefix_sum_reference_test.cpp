// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Formal_Power_Series.cpp"
#include "../../../Math/Polynomial_Prefix_Sum.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::uint64_t kMod = 998244353;

std::uint64_t evaluate(const sirkit::FormalPowerSeries &polynomial, std::uint64_t x)
{
    std::uint64_t result = 0;
    for (std::size_t i = polynomial.size(); i > 0; --i)
    {
        result = (result * x + polynomial.get(i - 1)) % kMod;
    }
    return result;
}
}

int main()
{
    using sirkit::FormalPowerSeries;
    using sirkit::PolynomialPrefixSum;
    assert(PolynomialPrefixSum(FormalPowerSeries{}) == FormalPowerSeries{0});
    assert(PolynomialPrefixSum(FormalPowerSeries{-5}) == (FormalPowerSeries{0, -5}));
    assert(PolynomialPrefixSum(FormalPowerSeries{0, 0, 0}) == (FormalPowerSeries{0, 0, 0, 0}));
    assert(PolynomialPrefixSum(FormalPowerSeries{0, 1}) ==
           (FormalPowerSeries{0, 499122176, 499122177}));
    std::mt19937 random(4673);
    for (std::size_t n : {1, 2, 3, 7, 16, 32, 33, 137, 1024})
    {
        for (int trial = 0; trial < 12; ++trial)
        {
            std::vector<std::int64_t> coefficients(n);
            for (auto &value : coefficients)
            {
                value = static_cast<std::int64_t>(random()) - 2'147'483'648LL;
            }
            const FormalPowerSeries f(coefficients);
            const auto g = PolynomialPrefixSum(f);
            assert(g.size() == n + 1 && g.get(0) == 0);
            std::uint64_t prefix = 0;
            for (std::uint64_t x = 0; x <= n && x < 160; ++x)
            {
                assert(prefix == evaluate(g, x));
                prefix = (prefix + evaluate(f, x)) % kMod;
            }
            for (int i = 0; i < 40; ++i)
            {
                const auto x = random() % kMod;
                assert((evaluate(g, x) + evaluate(f, x)) % kMod == evaluate(g, (x + 1) % kMod));
            }
        }
    }
    try
    {
        const FormalPowerSeries too_long(FormalPowerSeries::max_precision() + 1);
        PolynomialPrefixSum(too_long);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
}
