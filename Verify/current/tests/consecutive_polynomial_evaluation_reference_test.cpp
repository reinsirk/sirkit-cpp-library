// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/NTT.cpp"
#include "../../../Math/Consecutive_Polynomial_Evaluation.cpp"
#include <array>
#include <cassert>
#include <limits>
#include <random>

namespace
{
constexpr std::int64_t kMod = 998244353;

std::uint32_t evaluate(std::span<const std::uint32_t> coefficients, std::int64_t x)
{
    auto normalized = (x % kMod + kMod) % kMod;
    std::uint64_t result = 0;
    for (std::size_t i = coefficients.size(); i > 0; --i)
    {
        result = (result * normalized + coefficients[i - 1]) % kMod;
    }
    return static_cast<std::uint32_t>(result);
}

void compare(std::span<const std::uint32_t> coefficients, std::int64_t first, std::size_t m)
{
    std::vector<std::uint32_t> samples(coefficients.size());
    for (std::size_t i = 0; i < samples.size(); ++i)
    {
        samples[i] = evaluate(coefficients, i) + static_cast<std::uint32_t>((i % 4) * kMod);
    }
    const auto got = sirkit::evaluate_consecutive_polynomial(samples, first, m);
    assert(got.size() == m);
    for (std::size_t j = 0; j < m; ++j)
    {
        const auto point = first % kMod + static_cast<std::int64_t>(j);
        assert(got[j] == evaluate(coefficients, point));
    }
}
}

int main()
{
    compare({}, 0, 7);
    compare(std::array<std::uint32_t, 1>{9}, 3, 12);
    compare(std::array<std::uint32_t, 2>{0, 1}, -2, 6);
    compare(std::array<std::uint32_t, 2>{1, 1}, -2, 0);
    std::mt19937 random(324119);
    for (unsigned trial = 0; trial < 260; ++trial)
    {
        const auto n = random() % 40;
        const auto m = random() % 65;
        std::vector<std::uint32_t> coefficients(n);
        for (auto &value : coefficients)
        {
            value = random() % kMod;
        }
        compare(coefficients, static_cast<std::int64_t>(random() % 120) - 60, m);
        compare(coefficients, std::numeric_limits<std::int64_t>::max() - trial, m);
        compare(coefficients, std::numeric_limits<std::int64_t>::min() + trial, m);
    }
    auto reject = [](std::span<const std::uint32_t> samples, std::size_t m)
    {
        try
        {
            sirkit::evaluate_consecutive_polynomial(samples, 0, m);
            assert(false);
        }
        catch (const std::length_error &)
        {
        }
    };
    const auto beyond_limit = (std::size_t{1} << 20) + 1;
    reject(std::vector<std::uint32_t>(beyond_limit), 1);
    reject({}, beyond_limit);
}
