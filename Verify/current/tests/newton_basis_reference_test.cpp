// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Formal_Power_Series.cpp"
#include "../../../Math/Newton_Basis.cpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::uint64_t kMod = 998244353;
using Polynomial = std::vector<std::uint32_t>;

Polynomial expand(const Polynomial &coefficients, const Polynomial &points)
{
    Polynomial answer(coefficients.size()), basis{1};
    for (std::size_t i = 0; i < coefficients.size(); ++i)
    {
        for (std::size_t j = 0; j < basis.size(); ++j)
        {
            answer[j] = (answer[j] + std::uint64_t{basis[j]} * coefficients[i]) % kMod;
        }
        if (i == points.size())
        {
            break;
        }
        Polynomial next(basis.size() + 1);
        const auto negative = kMod - points[i] % kMod;
        for (std::size_t j = 0; j < basis.size(); ++j)
        {
            next[j] = (next[j] + negative * basis[j]) % kMod;
            next[j + 1] = basis[j];
        }
        basis.swap(next);
    }
    return answer;
}
}

int main()
{
    using sirkit::monomial_to_newton_basis;
    using sirkit::newton_to_monomial_basis;
    assert(monomial_to_newton_basis({}, {}).empty());
    assert(newton_to_monomial_basis({}, {}).empty());
    assert((newton_to_monomial_basis(Polynomial{0, 0, 0}, Polynomial{5, 5}) == Polynomial{0, 0, 0}));
    assert((monomial_to_newton_basis(Polynomial{1, 2, 1}, Polynomial{1, 2}) == Polynomial{4, 5, 1}));
    for (bool forward : {false, true})
    {
        const auto convert = forward ? monomial_to_newton_basis : newton_to_monomial_basis;
        try
        {
            static_cast<void>(convert(Polynomial{1, 1}, {}));
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
        try
        {
            static_cast<void>(convert(Polynomial((std::size_t{1} << 20) + 1), {}));
            assert(false);
        }
        catch (const std::length_error &)
        {
        }
    }
    std::mt19937 rng(314159265);
    for (unsigned test = 0; test < 620; ++test)
    {
        const std::size_t size = test < 100 ? test + 1 : 1 + rng() % 220;
        Polynomial coefficients(size), points(size - 1);
        for (auto &x : coefficients)
        {
            x = (test % 3 == 0) ? rng() % 9 : rng();
        }
        if (test % 5 == 0)
        {
            coefficients.back() = 0;
        }
        for (auto &x : points)
        {
            x = (test % 3 == 0) ? rng() % 3 : rng();
        }
        auto expected = expand(coefficients, points);
        assert(newton_to_monomial_basis(coefficients, points) == expected);
        for (auto &x : coefficients)
        {
            x %= kMod;
        }
        assert(monomial_to_newton_basis(expected, points) == coefficients);
    }
    const Polynomial coefficients{0, 0, std::numeric_limits<std::uint32_t>::max()};
    const Polynomial points{0, 0};
    assert(newton_to_monomial_basis(coefficients, points) == expand(coefficients, points));
    constexpr std::size_t kLarge = (std::size_t{1} << 14) + 1;
    Polynomial source(kLarge), zero_points(kLarge - 1);
    for (auto &x : source)
    {
        x = rng() % kMod;
    }
    assert(monomial_to_newton_basis(source, zero_points) == source);
    assert(newton_to_monomial_basis(source, zero_points) == source);
}
