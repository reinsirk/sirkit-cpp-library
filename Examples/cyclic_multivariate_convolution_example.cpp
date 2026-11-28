// SPDX-License-Identifier: CC0-1.0
#include "../Math/Cyclic_Multivariate_Convolution.cpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    const std::array<std::size_t, 2> dimensions{2, 3};
    const std::vector<std::uint32_t> a{1, 2, 3, 4, 5, 6}, shift{0, 0, 1, 0, 0, 0};
    const auto result = sirkit::cyclic_multivariate_convolution(dimensions, a, shift, 7);
    assert((result == std::vector<std::uint32_t>{5, 6, 1, 2, 3, 4}));
}
