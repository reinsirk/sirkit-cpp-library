// SPDX-License-Identifier: CC0-1.0
#include "../Math/Large_Convolution.cpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<std::uint32_t> a{1, 2, 3}, b{4, 5};
    assert((sirkit::convolution_mod_large(a, b) == std::vector<std::uint32_t>{4, 13, 22, 15}));
}
