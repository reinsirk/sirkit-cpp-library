// SPDX-License-Identifier: CC0-1.0
#include "Math/Pairwise_Coprime_Progression.cpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
bool prime(std::uint32_t value)
{
    for (std::uint32_t divisor = 2; divisor * divisor <= value; ++divisor)
    {
        if (value % divisor == 0)
        {
            return false;
        }
    }
    return true;
}

bool feasible(std::uint32_t length, std::uint64_t step)
{
    for (std::uint32_t modulus = 2; modulus < length; ++modulus)
    {
        if (!prime(modulus))
        {
            continue;
        }
        bool any_good = false;
        for (std::uint32_t start = 0; start < modulus; ++start)
        {
            std::uint32_t bad_positions = 0;
            for (std::uint32_t i = 0; i < length; ++i)
            {
                bad_positions += (start + i * (step % modulus)) % modulus == 0;
            }
            any_good |= bad_positions < 2;
        }
        if (!any_good)
        {
            return false;
        }
    }
    return true;
}

bool small_agrees(std::uint32_t length, std::uint64_t step)
{
    const auto witness = sirkit::pairwise_coprime_progression_start(length, step);
    if (witness.has_value() != feasible(length, step))
    {
        return false;
    }
    if (witness)
    {
        const auto start = std::stoull(witness->to_string());
        for (std::uint32_t i = 0; i < length; ++i)
        {
            for (std::uint32_t j = i + 1; j < length; ++j)
            {
                if (std::gcd(start + i * step, start + j * step) != 1)
                {
                    return false;
                }
            }
        }
    }
    return true;
}
}

int main()
{
    for (std::uint64_t step = 1; step <= 400; ++step)
    {
        for (std::uint32_t length = 0; length <= 17; ++length)
        {
            if (!small_agrees(length, step))
            {
                return 1;
            }
        }
    }
    constexpr std::uint64_t step = 614889782588491410;
    constexpr std::uint32_t length = 105;
    const auto start = sirkit::pairwise_coprime_progression_start(length, step);
    const auto too_long = sirkit::pairwise_coprime_progression_start(length + 1, step);
    if (!start || too_long || !feasible(length, step) || feasible(length + 1, step))
    {
        return 2;
    }
    const sirkit::BigInteger<10> delta(std::to_string(step));
    std::vector<sirkit::BigInteger<10>> terms{*start};
    for (std::uint32_t i = 1; i < length; ++i)
    {
        terms.push_back(terms.back() + delta);
    }
    for (std::size_t i = 0; i < terms.size(); ++i)
    {
        for (std::size_t j = 0; j < i; ++j)
        {
            auto a = terms[i], b = terms[j];
            while (b.sign() != 0)
            {
                auto rest = a % b;
                a = std::move(b);
                b = std::move(rest);
            }
            if (a != sirkit::BigInteger<10>(1))
            {
                return 3;
            }
        }
    }
    constexpr auto last_step = std::numeric_limits<std::uint64_t>::max();
    const auto last_start = sirkit::pairwise_coprime_progression_start(1, last_step);
    if (!last_start || last_start->to_string() != "18446744073709551616")
    {
        return 4;
    }
    try
    {
        static_cast<void>(sirkit::max_pairwise_coprime_progression_length(0));
        return 5;
    }
    catch (const std::invalid_argument &)
    {
    }
}
