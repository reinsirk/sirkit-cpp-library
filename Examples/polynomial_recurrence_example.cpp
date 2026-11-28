// SPDX-License-Identifier: CC0-1.0
#include "../Math/Polynomial_Recurrence.cpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    const std::array<std::int64_t, 1> seed{1};
    const std::vector<std::vector<std::int64_t>> recurrence{{-1, -1}, {1}};
    std::cout << sirkit::polynomial_recurrence_term(seed, recurrence, 5) << '\n'; // 120
}
