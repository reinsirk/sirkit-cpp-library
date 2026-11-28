// SPDX-License-Identifier: CC0-1.0
#include "../Math/Polynomial_Factorization.cpp"
#include <array>
#include <iostream>

int main()
{
    const std::array<std::uint32_t, 3> coefficients{1, 1, 1};       // x^2+x+1
    const auto result = sirkit::factor_polynomial(coefficients, 3); // (x+2)^2
    std::cout << result.factors.front().multiplicity << '\n';       // 2
}
