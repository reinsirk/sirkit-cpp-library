// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_GAUSSIAN_INTEGER_GCD_DEFINED
#include "../../../Math/Gaussian_Integer_GCD.cpp"
#endif
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>

namespace
{
using G = sirkit::GaussianInteger;

bool divides(G factor, G value)
{
    const auto norm = factor.real * factor.real + factor.imag * factor.imag;
    if (norm == 0)
    {
        return value == G{};
    }
    const auto scaled_real = value.real * factor.real + value.imag * factor.imag;
    const auto scaled_imag = value.imag * factor.real - value.real * factor.imag;
    return scaled_real % norm == 0 && scaled_imag % norm == 0;
}

G multiply(G a, G b)
{
    return {a.real * b.real - a.imag * b.imag, a.real * b.imag + a.imag * b.real};
}

G check(G a, G b)
{
    const auto result = sirkit::gaussian_gcd(a, b);
    assert(result == G{} || (result.real > 0 && result.imag >= 0));
    assert(divides(result, a) && divides(result, b));
    return result;
}
} // namespace

int main()
{
    assert((G{1, 2} != G{1, 3}));
    assert(check({}, {}) == G{});
    for (int ra = -3; ra <= 3; ++ra)
    {
        for (int ia = -3; ia <= 3; ++ia)
        {
            const G a{ra, ia};
            for (int rb = -3; rb <= 3; ++rb)
            {
                for (int ib = -3; ib <= 3; ++ib)
                {
                    const G b{rb, ib};
                    const auto result = check(a, b);
                    // 小さい入力の非零公約数はノルム18以下。すべての候補を直接確認。
                    for (int real = -4; real <= 4; ++real)
                    {
                        for (int imag = -4; imag <= 4; ++imag)
                        {
                            const G divisor{real, imag};
                            assert(divides(divisor, result) == (divides(divisor, a) && divides(divisor, b)));
                        }
                    }
                }
            }
        }
    }

    constexpr auto kLim = std::int64_t{std::numeric_limits<std::int32_t>::max()};
    assert((check({kLim, kLim}, {kLim, -kLim}) == G{kLim, kLim}));
    assert((check({-kLim, -kLim}, {kLim, kLim}) == G{kLim, kLim}));
    assert((check({kLim, kLim}, {kLim - 2, kLim - 2}) == G{1, 1}));
    assert((check({kLim, kLim}, {kLim - 1, kLim}) == G{1, 0}));
    assert((check({0, -kLim}, {}) == G{kLim, 0}));
    assert((check({}, {-kLim, 0}) == G{kLim, 0}));

    using sirkit::detail::GaussianRoundQuotient;
    assert(GaussianRoundQuotient(15, 6) == 3);
    assert(GaussianRoundQuotient(-15, 6) == -3);
    assert(GaussianRoundQuotient(3, 7) == 0);
    assert(GaussianRoundQuotient(-3, 7) == 0);
    assert(GaussianRoundQuotient(4, 7) == 1);
    assert(GaussianRoundQuotient(-4, 7) == -1);
    constexpr auto kBigDenom = std::numeric_limits<std::int64_t>::max();
    assert(GaussianRoundQuotient(kBigDenom / 2, kBigDenom) == 0);
    assert(GaussianRoundQuotient(kBigDenom / 2 + 1, kBigDenom) == 1);
    assert(GaussianRoundQuotient(-kBigDenom / 2 - 1, kBigDenom) == -1);

    std::mt19937 random(2288);
    std::uniform_int_distribution<int> factor(-1'000'000, 1'000'000);
    std::uniform_int_distribution<int> multiplier(-999, 999);
    for (int i = 0; i < 100'000; ++i)
    {
        const G expected{factor(random), factor(random)};
        const G u{multiplier(random), multiplier(random)};
        const G v{u.real + 1, u.imag};
        // uとu+1の差は1なので、共通因子を掛けた結果の最大公約数が分かる。
        const auto result = check(multiply(expected, u), multiply(expected, v));
        assert(divides(result, expected) && divides(expected, result));
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto bad : {G{-kLim - 1, 0}, G{kLim + 1, 0}, G{0, -kLim - 1}, G{0, kLim + 1}})
    {
        bool first_rejected = false, second_rejected = false;
        try
        {
            (void)sirkit::gaussian_gcd(bad, {});
        }
        catch (const std::out_of_range &)
        {
            first_rejected = true;
        }
        try
        {
            (void)sirkit::gaussian_gcd({}, bad);
        }
        catch (const std::out_of_range &)
        {
            second_rejected = true;
        }
        assert(first_rejected && second_rejected);
    }
#endif
}
