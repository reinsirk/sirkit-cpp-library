// SPDX-License-Identifier: CC0-1.0
#include "../Math/Mixed_Radix.cpp"
#include <array>
#include <iostream>

int main()
{
    const std::array<std::uint32_t, 3> radices{3, 4, 2};
    const std::array<std::uint32_t, 3> digits{1, 2, 1};
    const sirkit::MixedRadix format(radices);
    std::cout << format.encode(digits).to_string() << '\n'; // 13
    std::cout << format.decode(13)[0] << '\n';              // 1
}
