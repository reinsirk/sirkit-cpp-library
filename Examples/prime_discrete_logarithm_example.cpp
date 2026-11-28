// SPDX-License-Identifier: CC0-1.0
#include "../Math/Prime_Discrete_Logarithm.cpp"
#include <iostream>

int main()
{
    const sirkit::PrimeDiscreteLogarithm table(7, 3, 2);
    std::cout << table.log(2) << '\n'; // 2
    std::cout << table.log(6) << '\n'; // 3
}
