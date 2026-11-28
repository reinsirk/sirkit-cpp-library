// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Small_Prime_Stirling.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>

int main()
{
    using Stirling = sirkit::SmallPrimeStirlingNumbers;
    for (const std::uint32_t prime : {2u, 3u, 5u, 7u, 11u, 31u, 4999u})
    {
        const auto first = Stirling::first_kind(prime);
        const auto second = Stirling::second_kind(prime);
        assert(first.modulus() == prime && second.modulus() == prime);
        assert(first.get(0, 1) == 0 && second.get(0, 1) == 0);
        assert(first.get(0, 0) == 1 && second.get(0, 0) == 1);
        std::array<std::uint32_t, 351> first_naive{}, second_naive{};
        first_naive[0] = second_naive[0] = 1;
        for (std::uint32_t n = 1; n <= 350; ++n)
        {
            for (std::uint32_t k = n; k != 0; --k)
            {
                first_naive[k] = (first_naive[k - 1] + (prime - (n - 1) % prime) * first_naive[k]) % prime;
                second_naive[k] = (second_naive[k - 1] + k * second_naive[k]) % prime;
                assert(first.get(n, k) == first_naive[k]);
                assert(second.get(n, k) == second_naive[k]);
            }
            first_naive[0] = second_naive[0] = 0;
            assert(first.get(n, 0) == 0 && second.get(n, 0) == 0);
        }
        constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
        assert(first.get(kMax, kMax) == 1 && second.get(kMax, kMax) == 1);
        const auto blocks = kMax / prime;
        assert(first.get(blocks * prime, blocks) == ((blocks & 1) == 0 ? 1 : prime - 1));
        assert(first.get(blocks * prime, blocks + prime - 1) ==
               blocks % prime * ((blocks & 1) != 0 ? 1 : prime - 1) % prime);
        assert(second.get(kMax, 1) == 1 && second.get(kMax, 0) == 0);
    }
    for (const std::uint32_t invalid : {0u, 1u, 4u, 5001u})
    {
        try
        {
            static_cast<void>(Stirling::first_kind(invalid));
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
}
