// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Tetration_Mod.cpp"
#include <cassert>
#include <cstdint>
#include <limits>

int main()
{
    const auto multiply = [](std::uint64_t base, std::uint64_t exponent, std::uint32_t modulus)
    {
        std::uint64_t result = 1 % modulus;
        for (std::uint64_t i = 0; i < exponent; ++i)
        {
            result = result * base % modulus;
        }
        return result;
    };
    for (std::uint32_t mod = 1; mod <= 350; ++mod)
    {
        for (std::uint64_t b = 0; b <= 8; ++b)
        {
            assert(sirkit::tetration_mod(b, 0, mod) == 1 % mod);
            assert(sirkit::tetration_mod(b, 1, mod) == b % mod);
            assert(sirkit::tetration_mod(b, 2, mod) == multiply(b, b, mod));
            if (b <= 4)
            {
                auto exponent = multiply(b, b, 1000000);
                assert(sirkit::tetration_mod(b, 3, mod) == multiply(b, exponent, mod));
            }
        }
        assert(sirkit::tetration_mod(2, 4, mod) == 65536 % mod);
        assert(sirkit::tetration_mod(2, 5, mod) == multiply(2, 65536, mod));
    }
    constexpr auto kMaxHeight = std::numeric_limits<std::uint64_t>::max();
    assert(sirkit::tetration_mod(0, kMaxHeight, 13) == 0);
    assert(sirkit::tetration_mod(0, kMaxHeight - 1, 13) == 1);
    assert(sirkit::tetration_mod(5, kMaxHeight, 25) == 0);
    assert(sirkit::tetration_mod(kMaxHeight, kMaxHeight, UINT32_MAX) == 0);
    try
    {
        sirkit::tetration_mod(2, 1, 0);
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
}
