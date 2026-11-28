// SPDX-License-Identifier: CC0-1.0
#include "Math/Weighted_Taylor_Shifts.cpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::uint64_t kMod = 998244353;

std::uint64_t normalized(std::int64_t x)
{
    const auto residue = x % static_cast<std::int64_t>(kMod);
    return static_cast<std::uint64_t>(residue < 0 ? residue + static_cast<std::int64_t>(kMod) : residue);
}

[[nodiscard]] std::vector<std::uint32_t> brute(const std::vector<std::int64_t> &coefficients,
                                               const std::vector<std::int64_t> &shifts,
                                               const std::vector<std::int64_t> &weights)
{
    std::vector<std::uint32_t> result(coefficients.size());
    for (std::size_t i = 0; i < shifts.size() && !coefficients.empty(); ++i)
    {
        const auto a = normalized(shifts[i]);
        const auto w = normalized(weights[i]);
        std::vector<std::uint64_t> shifted(coefficients.size());
        for (std::size_t k = coefficients.size(); k != 0; --k)
        {
            for (std::size_t j = coefficients.size() - k; j != 0; --j)
            {
                shifted[j] = (shifted[j] * a + shifted[j - 1]) % kMod;
            }
            shifted[0] = (shifted[0] * a + normalized(coefficients[k - 1])) % kMod;
        }
        for (std::size_t j = 0; j < result.size(); ++j)
        {
            result[j] = static_cast<std::uint32_t>((result[j] + shifted[j] * w) % kMod);
        }
    }
    return result;
}

bool agrees(const std::vector<std::int64_t> &coefficients, const std::vector<std::int64_t> &shifts,
            const std::vector<std::int64_t> &weights)
{
    const auto got = sirkit::weighted_taylor_shifts(sirkit::FormalPowerSeries(coefficients), shifts, weights);
    return got.coefficients() == brute(coefficients, shifts, weights);
}
}

int main()
{
    if (!agrees({}, {1}, {2}) || !agrees({2, 5, 0}, {}, {}) || !agrees({-5}, {1, 1, -7}, {3, 2, -5}) ||
        !agrees({2, 1, 7, 3}, {std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max()}, {-1, 59}))
    {
        return 1;
    }
    std::mt19937 random(194775);
    const auto sample = [&random](std::vector<std::int64_t> &values)
    {
        for (auto &x : values)
        {
            x = static_cast<std::int64_t>(random()) - 2000000000;
        }
    };
    for (int test = 0; test < 150; ++test)
    {
        std::vector<std::int64_t> coefficients(random() % 63), shifts(random() % 66), weights(shifts.size());
        sample(coefficients);
        sample(shifts);
        sample(weights);
        if (!agrees(coefficients, shifts, weights))
        {
            return 2;
        }
    }
    const std::vector<int> no_shift, mismatch{1};
    try
    {
        static_cast<void>(sirkit::weighted_taylor_shifts(sirkit::FormalPowerSeries{}, mismatch, no_shift));
        return 3;
    }
    catch (const std::invalid_argument &)
    {
    }
    try
    {
        const sirkit::FormalPowerSeries over(sirkit::FormalPowerSeries::max_precision() + 1);
        static_cast<void>(sirkit::weighted_taylor_shifts(over, no_shift, no_shift));
        return 4;
    }
    catch (const std::length_error &)
    {
    }
}
