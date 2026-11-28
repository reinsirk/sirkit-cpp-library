// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Reeds_Sloane.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
std::uint32_t next(const std::vector<std::uint32_t> &history, std::size_t index,
                   const std::vector<std::uint32_t> &coefficients, std::uint32_t modulus)
{
    std::uint64_t value = 0;
    for (std::size_t j = 0; j < coefficients.size(); ++j)
    {
        value = (value + std::uint64_t{coefficients[j]} * (history[index - 1 - j] % modulus)) % modulus;
    }
    return static_cast<std::uint32_t>(value);
}

bool consistent(const std::vector<std::uint32_t> &sequence, const std::vector<std::uint32_t> &coefficients,
                std::uint32_t modulus)
{
    for (std::size_t i = coefficients.size(); i < sequence.size(); ++i)
    {
        if (sequence[i] % modulus != next(sequence, i, coefficients, modulus))
        {
            return false;
        }
    }
    return true;
}

std::size_t minimum_order(const std::vector<std::uint32_t> &sequence, std::uint32_t modulus)
{
    for (std::size_t order = 0; order < sequence.size(); ++order)
    {
        std::uint64_t limit = 1;
        for (std::size_t i = 0; i < order; ++i)
        {
            limit *= modulus;
        }
        std::vector<std::uint32_t> coefficients(order);
        for (std::uint64_t candidate = 0; candidate < limit; ++candidate)
        {
            auto digits = candidate;
            for (auto &coefficient : coefficients)
            {
                coefficient = static_cast<std::uint32_t>(digits % modulus);
                digits /= modulus;
            }
            if (consistent(sequence, coefficients, modulus))
            {
                return order;
            }
        }
    }
    return sequence.size();
}

void compare(const std::vector<std::uint32_t> &sequence, std::uint32_t modulus)
{
    const auto coefficients = sirkit::reeds_sloane(sequence, modulus);
    assert(std::ranges::all_of(coefficients, [modulus](auto x)
                               {
                                   return x < modulus;
                               }));
    assert(consistent(sequence, coefficients, modulus));
    assert(coefficients.size() == minimum_order(sequence, modulus));
}
} // namespace

int main()
{
    assert(sirkit::reeds_sloane({}, 12).empty());
    assert(sirkit::reeds_sloane(std::array<std::uint32_t, 3>{1, 2, 3}, 1).empty());
    assert(sirkit::reeds_sloane(std::array<std::uint32_t, 3>{40, 20, 0}, 20).empty());
    bool rejected = false;
    try
    {
        static_cast<void>(sirkit::reeds_sloane({}, 0));
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);
    for (std::uint32_t modulus = 2; modulus <= 6; ++modulus)
    {
        std::uint32_t count = 1;
        for (std::size_t length = 1; length <= 4; ++length)
        {
            count *= modulus;
            std::vector<std::uint32_t> sequence(length);
            for (std::uint32_t code = 0; code < count; ++code)
            {
                auto digits = code;
                for (auto &value : sequence)
                {
                    value = digits % modulus;
                    digits /= modulus;
                }
                compare(sequence, modulus);
            }
        }
    }

    std::mt19937_64 rng(20261128);
    constexpr auto kLimit = std::numeric_limits<std::uint32_t>::max();
    const std::array<std::uint32_t, 14> moduli{2, 3, 4, 8, 9, 12, 21, 72, 125, 1024, 1U << 31, kLimit,
                                               4294967291U, 1000000000U};
    for (const auto modulus : moduli)
    {
        for (unsigned i = 0; i < 35; ++i)
        {
            const std::size_t order = 1 + rng() % 10;
            std::vector<std::uint32_t> values(order), recurrence(order);
            for (std::size_t j = 0; j < order; ++j)
            {
                values[j] = static_cast<std::uint32_t>(rng());
                recurrence[j] = static_cast<std::uint32_t>(rng() % modulus);
            }
            while (values.size() < 6 * order + 5)
            {
                values.push_back(next(values, values.size(), recurrence, modulus));
            }
            const auto prefix = std::span{values}.first(2 * order + 1);
            const auto reconstructed = sirkit::reeds_sloane(prefix, modulus);
            assert(reconstructed.size() <= order && consistent(values, reconstructed, modulus));
        }
    }
    compare({2, 2, 2, 2, 2, 2}, 4);
    compare({6, 2, 6, 4, 7, 6}, 12);
    compare({2, 4, 1, 1, 7, 3}, 8);
}
