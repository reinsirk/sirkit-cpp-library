// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Polynomial_Recurrence.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::int64_t kMod = 998244353;
using Coefficients = std::vector<std::vector<std::int64_t>>;
using Initial = std::vector<std::int64_t>;

std::uint64_t Mod(std::int64_t value)
{
    return static_cast<std::uint64_t>((value % kMod + kMod) % kMod);
}

std::uint64_t Power(std::uint64_t base, std::uint64_t exponent)
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

std::uint64_t Eval(const std::vector<std::int64_t> &polynomial, std::uint64_t argument)
{
    std::uint64_t x = 1;
    std::uint64_t sum = 0;
    for (const auto coefficient : polynomial)
    {
        sum = (sum + x * Mod(coefficient)) % kMod;
        x = x * argument % kMod;
    }
    return sum;
}

std::uint64_t Direct(const Initial &initial, const Coefficients &coefficients, std::size_t index)
{
    std::vector<std::uint64_t> terms;
    for (const auto value : initial)
    {
        terms.push_back(Mod(value));
    }
    const auto order = initial.size();
    for (std::size_t n = order; n <= index; ++n)
    {
        const auto argument = n - order;
        const auto leading = Eval(coefficients.back(), argument);
        assert(leading != 0);
        std::uint64_t sum = 0;
        for (std::size_t j = 0; j < order; ++j)
        {
            sum = (sum + Eval(coefficients[j], argument) * terms[argument + j]) % kMod;
        }
        terms.push_back((kMod - sum) * Power(leading, kMod - 2) % kMod);
    }
    return terms[index];
}
} // namespace

int main()
{
    using sirkit::polynomial_recurrence_term;
    constexpr auto kLow = std::numeric_limits<std::int64_t>::min();
    constexpr auto kHigh = std::numeric_limits<std::int64_t>::max();
    constexpr auto kOff = std::numeric_limits<std::uint64_t>::max();
    const Initial unit{1};
    const Coefficients factorial{{-1, -1}, {1}};
    assert(polynomial_recurrence_term(unit, factorial, 0) == 1);
    assert(polynomial_recurrence_term(unit, factorial, 5) == 120);
    assert(polynomial_recurrence_term(unit, Coefficients{{-1, -1, kMod, 0}, {1, 0}}, 5) == 120);
    assert(polynomial_recurrence_term(unit, factorial, 383) == Direct(unit, factorial, 383));
    const Initial extreme{kLow};
    const Coefficients extreme_coefficients{{kLow, kHigh}, {kHigh}};
    assert(polynomial_recurrence_term(extreme, extreme_coefficients, 170) == Direct(extreme, extreme_coefficients, 170));

    const Coefficients sign{{1}, {1}};
    assert(polynomial_recurrence_term(Initial{3}, sign, kOff) == kMod - 3);
    const Coefficients constant_variable{{3, 0, -1}, {-3, 0, 1}};
    assert(Power(3, (kMod - 1) / 2) == kMod - 1);
    assert(polynomial_recurrence_term(Initial{37}, constant_variable, kOff) == 37);

    std::mt19937 random(690953);
    for (int trial = 0; trial < 330; ++trial)
    {
        const auto order = static_cast<std::size_t>(random() % 4 + 1);
        Initial initial(order);
        Coefficients coefficients(order + 1);
        for (std::size_t i = 0; i < order; ++i)
        {
            initial[i] = static_cast<std::int64_t>(random() % 80) - 40;
            coefficients[i].resize(random() % 7);
            for (auto &value : coefficients[i])
            {
                value = static_cast<std::int64_t>(random() % 60) - 30;
            }
        }
        coefficients.back() = {static_cast<std::int64_t>(random() % 50 + 1), trial % 7, trial % 2};
        const auto index = static_cast<std::size_t>(random() % 240);
        assert(polynomial_recurrence_term(initial, coefficients, index) == Direct(initial, coefficients, index));
    }

    for (const auto &bad : {Coefficients{}, Coefficients{{3}}, Coefficients{{1}, {2}, {3}}})
    {
        bool rejected = false;
        try
        {
            static_cast<void>(polynomial_recurrence_term(unit, bad, 0));
        }
        catch (const std::invalid_argument &)
        {
            rejected = true;
        }
        assert(rejected);
    }
    bool missing_seed = false;
    try
    {
        static_cast<void>(polynomial_recurrence_term({}, factorial, 0));
    }
    catch (const std::invalid_argument &)
    {
        missing_seed = true;
    }
    assert(missing_seed);

    bool missing_leader = false;
    try
    {
        static_cast<void>(polynomial_recurrence_term(unit, Coefficients{{1}, {-17, 1}}, 200));
    }
    catch (const std::domain_error &)
    {
        missing_leader = true;
    }
    assert(missing_leader);

    bool periodic_zero = false;
    try
    {
        static_cast<void>(polynomial_recurrence_term(unit, Coefficients{{1}, {1, 1}}, kMod));
    }
    catch (const std::domain_error &)
    {
        periodic_zero = true;
    }
    assert(periodic_zero);
}
