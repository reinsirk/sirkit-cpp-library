// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Tridiagonal_Matrix_Power.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;
using Coefficients = std::vector<std::uint32_t>;
using Diagonal = std::vector<std::int64_t>;
using Matrix = std::vector<Coefficients>;

std::uint32_t Residue(std::int64_t value)
{
    return static_cast<std::uint32_t>((value % kMod + kMod) % kMod);
}

Matrix Multiply(const Matrix &a, const Matrix &b)
{
    const auto n = a.size();
    Matrix result(n, Coefficients(n));
    for (std::size_t i = 0; i < n; ++i)
    {
        for (std::size_t j = 0; j < n; ++j)
        {
            for (std::size_t k = 0; k < n; ++k)
            {
                result[i][j] = static_cast<std::uint32_t>((result[i][j] + std::uint64_t{a[i][k]} * b[k][j]) % kMod);
            }
        }
    }
    return result;
}

Coefficients DensePower(std::uint64_t power, const Diagonal &lower, const Diagonal &diagonal, const Diagonal &upper)
{
    const auto n = diagonal.size();
    Matrix current(n, Coefficients(n)), raised(n, Coefficients(n));
    for (std::size_t i = 0; i < n; ++i)
    {
        current[i][i] = Residue(diagonal[i]);
        raised[i][i] = 1;
    }
    for (std::size_t i = 0; i < lower.size(); ++i)
    {
        current[i + 1][i] = Residue(lower[i]);
        current[i][i + 1] = Residue(upper[i]);
    }
    while (power != 0)
    {
        if ((power & 1u) != 0)
        {
            raised = Multiply(raised, current);
        }
        current = Multiply(current, current);
        power >>= 1;
    }
    Coefficients result;
    for (const auto &row : raised)
    {
        result.push_back(row.front());
    }
    return result;
}

Coefficients Simulate(std::size_t steps, const Diagonal &lower, const Diagonal &diagonal, const Diagonal &upper)
{
    const auto n = diagonal.size();
    Coefficients now(n), next(n);
    now[0] = 1;
    for (std::size_t step = 0; step < steps; ++step)
    {
        for (std::size_t i = 0; i < n; ++i)
        {
            auto value = std::uint64_t{now[i]} * Residue(diagonal[i]);
            if (i > 0)
            {
                value += std::uint64_t{now[i - 1]} * Residue(lower[i - 1]);
            }
            if (i + 1 < n)
            {
                value += std::uint64_t{now[i + 1]} * Residue(upper[i]);
            }
            next[i] = static_cast<std::uint32_t>(value % kMod);
        }
        now.swap(next);
    }
    return now;
}
} // namespace

int main()
{
    using sirkit::tridiagonal_power_first_column;
    assert(tridiagonal_power_first_column(10, {}, {}, {}).empty());
    const Diagonal lower{1, 2, 3}, upper{3, 2, 1}, diagonal{5, -7, 0, 9};
    assert(tridiagonal_power_first_column(0, lower, diagonal, upper) == Coefficients({1, 0, 0, 0}));
    assert(tridiagonal_power_first_column(1, lower, diagonal, upper) == Coefficients({5, 1, 0, 0}));
    assert(tridiagonal_power_first_column(3, lower, diagonal, upper) == Simulate(3, lower, diagonal, upper));

    constexpr auto kHigh = std::numeric_limits<std::int64_t>::max();
    constexpr auto kLow = std::numeric_limits<std::int64_t>::min();
    constexpr auto kHuge = std::numeric_limits<std::uint64_t>::max();
    const std::array<std::uint32_t, 1> seed{1}, factor{3};
    const Diagonal geometric{3};
    assert(tridiagonal_power_first_column(kHuge, {}, geometric, {}) ==
           sirkit::consecutive_linear_recurrence(seed, factor, kHuge, 1));
    const Diagonal nil{0}, huge_diagonal{kLow, kHigh}, huge_up{kLow}, huge_down{kHigh};
    assert(tridiagonal_power_first_column(kHuge, {}, nil, {}) == Coefficients({0}));
    assert(tridiagonal_power_first_column(kHuge, huge_down, huge_diagonal, huge_up) ==
           DensePower(kHuge, huge_down, huge_diagonal, huge_up));
    const Diagonal broken_lower{1, kMod, 2, 1}, broken_upper{1, 0, 1, 2}, broken_diagonal{0, 0, 0, 3, -9};
    assert(tridiagonal_power_first_column(kHuge, broken_lower, broken_diagonal, broken_upper) ==
           DensePower(kHuge, broken_lower, broken_diagonal, broken_upper));

    std::mt19937_64 random(189016);
    for (int trial = 0; trial < 330; ++trial)
    {
        const auto n = static_cast<std::size_t>(random() % 11 + 1);
        Diagonal a(n - 1), b(n), c(n - 1);
        for (auto *line : {&a, &b, &c})
        {
            for (auto &value : *line)
            {
                value = static_cast<std::int64_t>(random() % 29) - 14;
            }
        }
        const auto power = trial % 2 == 0 ? random() : random() % 25;
        assert(tridiagonal_power_first_column(power, a, b, c) == DensePower(power, a, b, c));
    }
    for (std::size_t n : {127u, 128u, 129u, 255u, 260u})
    {
        Diagonal a(n - 1), b(n), c(n - 1);
        for (auto *line : {&a, &b, &c})
        {
            for (auto &value : *line)
            {
                value = static_cast<std::int64_t>(random() % 10001 + 1);
            }
        }
        for (auto k : {n - 5, n + 1, 4 * n})
        {
            assert(tridiagonal_power_first_column(k, a, b, c) == Simulate(k, a, b, c));
        }
    }

    bool mismatch = false;
    try
    {
        static_cast<void>(tridiagonal_power_first_column(0, {}, diagonal, upper));
    }
    catch (const std::invalid_argument &)
    {
        mismatch = true;
    }
    assert(mismatch);
    mismatch = false;
    try
    {
        static_cast<void>(tridiagonal_power_first_column(0, lower, diagonal, {}));
    }
    catch (const std::invalid_argument &)
    {
        mismatch = true;
    }
    assert(mismatch);
    constexpr std::size_t kSide = (std::size_t{1} << 20) + 1;
    const Diagonal too_large(kSide, 1);
    const auto short_view = std::span<const std::int64_t>(too_large).first(kSide - 1);
    bool limited = false;
    try
    {
        static_cast<void>(tridiagonal_power_first_column(kSide, short_view, too_large, short_view));
    }
    catch (const std::length_error &)
    {
        limited = true;
    }
    assert(limited);
}
