// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Chromatic_Polynomial.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}};
    const auto coefficients = sirkit::chromatic_polynomial(3, edges);
    assert((coefficients == std::vector<std::uint32_t>{0, 2, 998244350, 1}));
}
