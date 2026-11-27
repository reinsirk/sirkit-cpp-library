// SPDX-License-Identifier: CC0-1.0
#include "../Math/Discrete_Logarithm.cpp"
#include <iostream>

int main()
{
    if (const auto exponent = sirkit::discrete_logarithm_mod(2, 4, 12))
    {
        std::cout << *exponent << '\n'; // 2
    }
}
