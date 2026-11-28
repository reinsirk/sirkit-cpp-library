// SPDX-License-Identifier: CC0-1.0
#include "../Math/Small_Prime_Stirling.cpp"
#include <iostream>

int main()
{
    const auto first = sirkit::SmallPrimeStirlingNumbers::first_kind(7);
    const auto second = sirkit::SmallPrimeStirlingNumbers::second_kind(7);
    std::cout << first.get(4, 2) << '\n';  // 11 mod 7 = 4
    std::cout << second.get(4, 2) << '\n'; // 7 mod 7 = 0
}
