// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/NTT.cpp"
#include "../../../Math/Geometric_Polynomial.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::uint64_t kMod = 998244353;
using Polynomial = std::vector<std::uint32_t>;

std::uint64_t power(std::uint64_t base, std::uint64_t exponent)
{
    std::uint64_t result = 1;
    for (; exponent != 0; exponent /= 2)
    {
        if (exponent % 2 != 0)
        {
            result = result * base % kMod;
        }
        base = base * base % kMod;
    }
    return result;
}

Polynomial reference(std::span<const std::uint32_t> polynomial, std::uint64_t a, std::uint64_t r, std::size_t count)
{
    Polynomial result(count);
    a %= kMod;
    r %= kMod;
    for (auto &value : result)
    {
        std::uint64_t sum = 0, term = 1;
        for (auto coefficient : polynomial)
        {
            sum = (sum + term * (coefficient % kMod)) % kMod;
            term = term * a % kMod;
        }
        value = static_cast<std::uint32_t>(sum);
        a = a * r % kMod;
    }
    return result;
}

template <class Operation>
void expect_length_error(Operation run)
{
    try
    {
        run();
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
}
}

int main()
{
    using sirkit::evaluate_geometric_polynomial;
    using sirkit::interpolate_geometric_polynomial;
    assert(evaluate_geometric_polynomial(Polynomial{1, 2}, 1, 1, 0).empty());
    assert((evaluate_geometric_polynomial({}, 1, 2, 3) == Polynomial{0, 0, 0}));
    const Polynomial p{998244352, 998244352, 4};
    for (std::uint64_t a : {0, 1, 2, 11})
    {
        for (std::uint64_t ratio : {0, 1, 2, 14})
        {
            assert(evaluate_geometric_polynomial(p, a + kMod, ratio + kMod, 15) == reference(p, a, ratio, 15));
        }
    }
    assert(interpolate_geometric_polynomial({}, 0, 0) == Polynomial{});
    assert((interpolate_geometric_polynomial(Polynomial{998244366}, 0, 0) == Polynomial{13}));
    assert((interpolate_geometric_polynomial(Polynomial{9, 5}, 2, 0) == Polynomial{5, 2}));
    assert(!interpolate_geometric_polynomial(p, 0, 2));
    assert(!interpolate_geometric_polynomial(p, 2, 0));
    assert(!interpolate_geometric_polynomial(p, 2, 1));

    std::mt19937 rng(148825);
    for (unsigned i = 0; i < 650; ++i)
    {
        const std::size_t n = rng() % 130 + 1;
        Polynomial coefficient(n);
        for (auto &x : coefficient)
        {
            x = rng() % kMod;
        }
        const auto a = std::uint64_t{1} + rng() % (kMod - 1);
        const auto ratio = power(3, 1 + rng() % 100);
        const auto values = reference(coefficient, a, ratio, n);
        assert(evaluate_geometric_polynomial(coefficient, a, ratio, n) == values);
        assert(interpolate_geometric_polynomial(values, a, ratio) == coefficient);
    }

    for (std::size_t n : {2u, 4u, 8u, 256u, 4096u})
    {
        const auto ratio = power(3, (kMod - 1) / n);
        Polynomial original(n);
        for (auto &x : original)
        {
            x = rng() % kMod;
        }
        auto samples = reference(original, 1, ratio, n + 1);
        assert(!interpolate_geometric_polynomial(samples, 1, ratio));
        samples.pop_back();
        assert(interpolate_geometric_polynomial(samples, 1, ratio) == original);
    }

    constexpr std::size_t kTooMany = (std::size_t{1} << 20) + 1;
    const Polynomial oversized(kTooMany);
    expect_length_error([&]
                        {
                            static_cast<void>(evaluate_geometric_polynomial(oversized, 0, 0, 2));
                        });
    expect_length_error([&]
                        {
                            static_cast<void>(evaluate_geometric_polynomial(p, 0, 0, kTooMany));
                        });
    expect_length_error([&]
                        {
                            static_cast<void>(interpolate_geometric_polynomial(oversized, 0, 0));
                        });
}
