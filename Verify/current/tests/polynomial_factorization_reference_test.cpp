// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Polynomial_Factorization.cpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>

namespace
{
using Coefficients = std::vector<std::uint32_t>;

Coefficients Product(const Coefficients &a, const Coefficients &b, std::uint32_t prime)
{
    Coefficients result(a.size() + b.size() - 1);
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        for (std::size_t j = 0; j < b.size(); ++j)
        {
            const auto product = std::uint64_t{a[i]} * b[j] % prime;
            result[i + j] = static_cast<std::uint32_t>((result[i + j] + product) % prime);
        }
    }
    return result;
}

bool Divisible(Coefficients value, const Coefficients &monic, std::uint32_t prime)
{
    const auto degree = monic.size() - 1;
    for (std::size_t i = value.size(); i-- > degree;)
    {
        const auto multiple = value[i];
        for (std::size_t j = 0; j <= degree; ++j)
        {
            const auto product = std::uint64_t{multiple} * monic[j] % prime;
            auto &coefficient = value[i - degree + j];
            coefficient = static_cast<std::uint32_t>((std::uint64_t{coefficient} + prime - product) % prime);
        }
    }
    return std::ranges::all_of(value, [](auto c)
                               {
                                   return c == 0;
                               });
}

bool IsIrreducible(const Coefficients &value, std::uint32_t prime)
{
    std::uint32_t count = 1;
    for (std::size_t d = 1; d <= (value.size() - 1) / 2; ++d)
    {
        count *= prime;
        Coefficients candidate(d + 1);
        candidate.back() = 1;
        for (std::uint32_t mask = 0; mask < count; ++mask)
        {
            auto digits = mask;
            for (std::size_t i = 0; i < d; ++i)
            {
                candidate[i] = digits % prime;
                digits /= prime;
            }
            if (Divisible(value, candidate, prime))
            {
                return false;
            }
        }
    }
    return true;
}

void CheckKnown(std::uint32_t prime, std::uint32_t leading, const std::map<Coefficients, std::size_t> &factors)
{
    Coefficients original{leading};
    for (const auto &[factor, multiplicity] : factors)
    {
        for (std::size_t i = 0; i < multiplicity; ++i)
        {
            original = Product(original, factor, prime);
        }
    }
    const auto result = sirkit::factor_polynomial(original, prime, 14737);
    assert(result.leading_coefficient == leading);
    assert(result.factors.size() == factors.size());
    for (const auto &factor : result.factors)
    {
        assert(factors.at(factor.coefficients) == factor.multiplicity);
    }
}
}

int main()
{
    using sirkit::factor_polynomial;
    const auto unit = factor_polynomial(Coefficients{10, 14}, 7, 38127);
    assert(unit.leading_coefficient == 3 && unit.factors.empty());
    try
    {
        static_cast<void>(factor_polynomial(Coefficients{7, 14}, 7, 21));
        assert(false);
    }
    catch (const std::domain_error &)
    {
    }
    for (const auto invalid : {0u, 1u, 4u})
    {
        try
        {
            static_cast<void>(factor_polynomial(Coefficients{1}, invalid, 21));
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
    CheckKnown(2, 1, {{{1, 1, 0, 1}, 3}, {{1, 0, 1, 1}, 5}});
    CheckKnown(2, 1, {{{1, 1, 0, 1}, 7}, {{1, 0, 1, 1}, 4}});
    CheckKnown(3, 2, {{{1, 0, 1}, 4}, {{2, 1, 1}, 4}, {{1, 1}, 3}});
    constexpr std::uint32_t kLarge = 4294967291U;
    CheckKnown(kLarge, kLarge - 1, {{{0, 1}, 2}, {{kLarge - 1, 1}, 4}, {{kLarge - 4, 1}, 3}});
    std::mt19937 random(15229);
    for (const auto prime : {2u, 3u, 5u, 7u})
    {
        for (std::size_t degree = 1; degree <= 9; ++degree)
        {
            for (int trial = 0; trial < 25; ++trial)
            {
                Coefficients value(degree + 1);
                for (auto &coefficient : value)
                {
                    coefficient = static_cast<std::uint32_t>(random() % prime);
                }
                value.back() = static_cast<std::uint32_t>(1 + random() % (prime - 1));
                const auto result = factor_polynomial(value, prime, random());
                assert(result.leading_coefficient == value.back());
                Coefficients rebuilt{result.leading_coefficient};
                std::set<Coefficients> seen;
                for (const auto &factor : result.factors)
                {
                    assert(seen.insert(factor.coefficients).second);
                    assert(factor.coefficients.back() == 1 && factor.multiplicity > 0);
                    assert(IsIrreducible(factor.coefficients, prime));
                    for (std::size_t i = 0; i < factor.multiplicity; ++i)
                    {
                        rebuilt = Product(rebuilt, factor.coefficients, prime);
                    }
                }
                assert(rebuilt == value);
            }
        }
    }
}
