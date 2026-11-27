// SPDX-License-Identifier: CC0-1.0
#include "../Math/Multivariate_Convolution.cpp"
#include <cassert>

int main()
{
    const std::vector<std::size_t> dimensions{2, 2};
    const std::vector<std::uint32_t> a{1, 2, 3, 4}, b{5, 6, 7, 8};
    assert((sirkit::multivariate_convolution(dimensions, a, b) == std::vector<std::uint32_t>{5, 16, 22, 60}));
}
