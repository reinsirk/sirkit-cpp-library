// SPDX-License-Identifier: CC0-1.0
#include "../Math/Consecutive_Linear_Recurrence.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::array<std::uint32_t, 2> initial{0, 1}, coefficients{1, 1};
    const auto result = sirkit::consecutive_linear_recurrence(initial, coefficients, 5, 4);
    assert((result == std::vector<std::uint32_t>{5, 8, 13, 21}));
}
