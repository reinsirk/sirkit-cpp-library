// SPDX-License-Identifier: CC0-1.0
#include "../Math/Combination/Q_Binomial.cpp"
#include <iostream>

int main()
{
    const sirkit::PrimeQBinomial coefficients(998244353, 2, 6);
    std::cout << coefficients.COM(6, 2) << '\n'; // 651
}
