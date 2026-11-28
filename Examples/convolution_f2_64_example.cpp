// SPDX-License-Identifier: CC0-1.0
#include "../Math/Convolution_F2_64.cpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::array<std::uint64_t, 2> a{1, 2}, b{1, 3};
    assert((sirkit::convolution_f2_64(a, b) == std::vector<std::uint64_t>{1, 1, 6}));
    assert(sirkit::multiply_f2_64(std::uint64_t{1} << 63, 2) == 27);
}
