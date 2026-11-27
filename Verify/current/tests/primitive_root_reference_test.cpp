// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Pollard_Rho.cpp"
#include "../../../Math/Primitive_Root.cpp"
#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace
{
bool small_prime(unsigned value)
{
    if (value < 2)
    {
        return false;
    }
    for (unsigned divisor = 2; divisor * divisor <= value; ++divisor)
    {
        if (value % divisor == 0)
        {
            return false;
        }
    }
    return true;
}

unsigned reference_order(unsigned a, unsigned prime)
{
    a %= prime;
    if (a == 0)
    {
        return 0;
    }
    unsigned order = 1;
    for (unsigned current = a; current != 1; current = current * a % prime)
    {
        ++order;
    }
    return order;
}
} // namespace

int main()
{
    for (unsigned p = 2; p < 450; ++p)
    {
        if (!small_prime(p))
        {
            continue;
        }
        const sirkit::PrimeMultiplicativeGroup group(p);
        assert(group.modulus() == p);
        unsigned first_generator = 0;
        for (unsigned a = 0; a <= 2 * p; ++a)
        {
            const unsigned expected = reference_order(a, p);
            assert(group.order(a).value_or(0) == expected);
            assert(group.is_generator(a) == (expected == p - 1));
            if (first_generator == 0 && expected == p - 1)
            {
                first_generator = a;
            }
        }
        assert(sirkit::primitive_root(p) == first_generator);
    }

    // p-1 = 2^2*11*137*547*5594472617641.
    constexpr std::uint64_t kLargePrime = 18446744073709551557ULL;
    const sirkit::PrimeMultiplicativeGroup group(kLargePrime);
    assert(group.primitive_root() == 2);
    assert(group.order(3) == kLargePrime - 1);
    assert(group.order(kLargePrime - 1) == 2);
    assert(group.order(kLargePrime + 1) == 1);
    assert(!group.order(kLargePrime));
    assert(group.is_generator(7));
    assert(!group.is_generator(kLargePrime - 1));
    for (std::uint64_t invalid : {0, 1, 4, 121})
    {
        try
        {
            const sirkit::PrimeMultiplicativeGroup group_invalid(invalid);
            assert(false);
        }
        catch (const std::domain_error &)
        {
        }
    }
}
