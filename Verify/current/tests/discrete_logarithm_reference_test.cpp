// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_DISCRETE_LOGARITHM_TEST_SOURCE
#define SIRKIT_DISCRETE_LOGARITHM_TEST_SOURCE "../../../Math/Discrete_Logarithm.cpp"
#endif
#include SIRKIT_DISCRETE_LOGARITHM_TEST_SOURCE
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

int main()
{
    for (std::uint32_t modulus = 1; modulus <= 96; ++modulus)
    {
        for (std::uint32_t base = 0; base < modulus; ++base)
        {
            std::vector<std::optional<std::uint32_t>> reference(modulus);
            auto value = 1U % modulus;
            for (std::uint32_t exponent = 0; !reference[value]; ++exponent)
            {
                reference[value] = exponent;
                value = (value * base) % modulus;
            }
            for (std::uint32_t target = 0; target < modulus; ++target)
            {
                assert(sirkit::discrete_logarithm_mod(base, target, modulus) == reference[target]);
                assert(sirkit::discrete_logarithm_mod(base + modulus, target + modulus, modulus) == reference[target]);
            }
        }
    }
    constexpr auto kMax = std::numeric_limits<std::uint32_t>::max();
    assert(sirkit::discrete_logarithm_mod(0, 0, kMax) == 1);
    assert(sirkit::discrete_logarithm_mod(kMax - 1, kMax - 1, kMax) == 1);
    assert(sirkit::discrete_logarithm_mod(kMax - 1, 2, kMax) == std::nullopt);
    assert(sirkit::discrete_logarithm_mod(2, 0, std::uint32_t{1} << 31) == 31);
    // 4294967291は素数で、2は原始根。-1が最初に現れるのは位数の半分。
    constexpr std::uint32_t kPrime = 4'294'967'291U;
    assert(sirkit::discrete_logarithm_mod(2, kPrime - 1, kPrime) == (kPrime - 1) / 2);
#ifndef SKIP_RUNTIME_CHECKS
    try
    {
        sirkit::discrete_logarithm_mod(1, 1, 0);
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
