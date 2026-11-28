// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <numeric>
#include <utility>

namespace
{
std::uint64_t Power(std::uint64_t base, std::uint64_t exponent, std::uint64_t modulus)
{
    std::uint64_t result = 1;
    for (; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1U) != 0)
        {
            result = result * base % modulus;
        }
        base = base * base % modulus;
    }
    return result;
}

std::uint64_t Inverse(std::uint64_t value, std::uint64_t modulus)
{
    auto divisor = static_cast<std::int64_t>(modulus);
    auto remainder = static_cast<std::int64_t>(value % modulus);
    std::int64_t coefficient = 0, next_coefficient = 1;
    while (remainder != 0)
    {
        const auto quotient = divisor / remainder;
        const auto next = divisor % remainder;
        divisor = std::exchange(remainder, next);
        coefficient = std::exchange(next_coefficient, coefficient - quotient * next_coefficient);
    }
    const auto signed_modulus = static_cast<std::int64_t>(modulus);
    return static_cast<std::uint64_t>((coefficient % signed_modulus + signed_modulus) % signed_modulus);
}

struct Geometric
{
    std::uint64_t multiplier;
    std::uint64_t sum;
};

std::uint64_t CoprimePart(std::uint32_t n, std::uint32_t m, std::uint64_t a, std::uint64_t b,
                          std::uint64_t base, std::uint64_t modulus)
{
    if (modulus == 1)
    {
        return 0;
    }
    const auto combine = [modulus](Geometric left, Geometric right)
    {
        return Geometric{left.multiplier * right.multiplier % modulus,
                         (left.sum + left.multiplier * right.sum) % modulus};
    };
    const Geometric horizontal{Power(base % modulus, a, modulus), 1};
    const Geometric vertical{Power(Inverse(base, modulus), m, modulus), 0};
    const auto word = sirkit::floor_path_product(n, m, a, b, horizontal, vertical, Geometric{1, 0}, combine);
    return word.sum * Power(base % modulus, b, modulus) % modulus;
}

std::uint64_t NilpotentPart(std::uint32_t n, std::uint32_t m, std::uint64_t a, std::uint64_t b,
                            std::uint64_t base, std::uint64_t modulus)
{
    const std::uint64_t gcd = std::gcd(a, std::uint64_t{m});
    const auto period = m / gcd;
    const auto inverse = Inverse(a / gcd, period);
    std::uint64_t answer = 0;
    // baseの冪が0になるまでに現れる剰余だけ数える。
    for (std::uint64_t r = 0, power = 1; r < m && power != 0; ++r, power = power * base % modulus)
    {
        const auto difference = (m + r - b) % m;
        if (difference % gcd == 0)
        {
            const auto first = (difference / gcd) * inverse % period;
            if (first < n)
            {
                const auto occurrences = 1 + (n - 1 - first) / period;
                answer = (answer + power * occurrences) % modulus;
            }
        }
    }
    return answer;
}
}

int main()
{
    int queries{};
    verify_in >> queries;
    while (queries-- > 0)
    {
        std::uint32_t n{}, m{};
        std::uint64_t a{}, b{}, base{}, modulus{};
        verify_in >> n >> m >> a >> b >> base >> modulus;
        auto unit = modulus;
        for (auto shared = std::gcd(unit, base); shared > 1; shared = std::gcd(unit, base))
        {
            unit /= shared;
        }
        const auto nilpotent = modulus / unit;
        const auto residue = CoprimePart(n, m, a, b, base, unit);
        if (nilpotent == 1)
        {
            verify_out << residue << '\n';
        }
        else
        {
            const auto other = NilpotentPart(n, m, a, b, base, nilpotent);
            const auto difference = (other + nilpotent - residue % nilpotent) % nilpotent;
            const auto offset = difference * Inverse(unit, nilpotent) % nilpotent;
            verify_out << residue + unit * offset << '\n';
        }
    }
}
