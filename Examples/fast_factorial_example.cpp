// SPDX-License-Identifier: CC0-1.0
#include "../Math/Fast_Factorial.cpp"
#include <array>
#include <iostream>

int main()
{
    std::cout << sirkit::factorial_mod_998244353(5) << '\n'; // 120
    const std::array<std::uint64_t, 3> queries{0, 10, 998244353};
    for (const auto answer : sirkit::factorials_mod_998244353(queries))
    {
        std::cout << answer << '\n'; // 1, 3628800, 0
    }
}
