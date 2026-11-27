// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Monge_Optimization.cpp"
#include <cassert>
#include <cstdlib>
#include <vector>

int main()
{
    const auto columns = sirkit::smawk(3, 4, [](int row, int current, int candidate)
                                       {
                                           return std::abs(row - candidate) < std::abs(row - current);
                                       });
    assert((columns == std::vector<int>{0, 1, 2}));

    const std::vector<int> a{0, 1, 4}, b{3, 0, 1};
    const auto sums = sirkit::min_plus_convolution_convex_convex(a, b);
    assert((sums == std::vector<int>{3, 0, 1, 2, 5}));
}
