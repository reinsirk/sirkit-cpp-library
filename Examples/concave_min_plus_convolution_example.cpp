// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Concave_Min_Plus_Convolution.cpp"
#include <cassert>

int main()
{
    const std::vector<std::int64_t> a{1, 4, 5}, b{0, 2};
    const auto c = sirkit::min_plus_convolution_concave_arbitrary(a, b);
    assert((c == std::vector<std::int64_t>{1, 3, 5, 7}));
}
