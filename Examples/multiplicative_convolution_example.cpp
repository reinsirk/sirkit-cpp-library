// SPDX-License-Identifier: CC0-1.0
#include "../Math/Multiplicative_Convolution.cpp"
#include <cassert>

int main()
{
    const std::vector<std::uint32_t> a{1, 2, 3}, b{4, 5, 6};
    assert((sirkit::multiplicative_convolution_prime(a, b) == std::vector<std::uint32_t>{35, 28, 27}));
}
