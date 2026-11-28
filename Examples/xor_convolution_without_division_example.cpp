// SPDX-License-Identifier: CC0-1.0
#include "../Math/Xor_Convolution_Without_Division.cpp"

#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    // uint64_t なら各演算の結果は 2^64 を法とします。
    using Coefficients = std::vector<std::uint64_t>;
    const auto result = sirkit::xor_convolution_without_division(Coefficients{2, 3}, Coefficients{4, 5});
    std::cout << result[0] << ' ' << result[1] << '\n'; // 23 22
}
