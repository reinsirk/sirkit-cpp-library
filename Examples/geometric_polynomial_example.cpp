// SPDX-License-Identifier: CC0-1.0
#include "../Math/Geometric_Polynomial.cpp"
#include <cassert>

int main()
{
    std::vector<std::uint32_t> f{1, 2, 1};
    const auto y = sirkit::evaluate_geometric_polynomial(f, 2, 3, 3);
    assert((y == std::vector<std::uint32_t>{9, 49, 361}));
    assert(sirkit::interpolate_geometric_polynomial(y, 2, 3) == f);
}
