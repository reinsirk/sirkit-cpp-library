// SPDX-License-Identifier: CC0-1.0
#include "../String/DFA_Minimization.cpp"

#include <array>
#include <cstdint>
#include <iostream>

int main()
{
    // 0と1は同じ働きで、2は到達しない。
    const std::array<std::size_t, 3> edges{1, 0, 2};
    const std::array<std::uint8_t, 3> accepting{1, 1, 0};
    const auto result = sirkit::minimize_dfa(1, edges, accepting);
    std::cout << result.size() << '\n'; // 1
}
