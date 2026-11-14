// SPDX-License-Identifier: CC0-1.0
#include "../Math/Modular_Arithmetic.cpp"
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>

using namespace sirkit;
int main() {
    constexpr auto power = ModPow(3, 2, std::uint64_t{1} << 32);
    static_assert(power == 9);
    constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
    std::cout << NormalizeMod(-7, 11) << '\n'; // 4
    std::cout << ModAdd(maximum - 1, 5, maximum) << '\n'; // 4
    std::cout << ModSubtract(2, 5, 7) << '\n'; // 4
    std::cout << ModMultiply(maximum - 1, maximum - 1, maximum) << '\n'; // 1
    std::cout << ModPow(2, 10, 1000) << '\n'; // 24
    if (auto inverse = ModInverse(3, 11)) std::cout << *inverse << '\n'; // 4
    if (auto equation = SolveLinearCongruence(4, 6, 10))
        std::cout << equation->remainder << ' ' << equation->modulus << '\n'; // 4 5
    const std::array<Congruence, 3> equations{{{2, 6}, {8, 9}, {5, 7}}};
    if (auto joined = ChineseRemainder(equations))
        std::cout << joined->remainder << ' ' << joined->modulus << '\n'; // 26 126
    if (auto root = ModSqrt(10, 13)) std::cout << *root << '\n'; // 6
}
