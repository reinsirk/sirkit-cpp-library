// SPDX-License-Identifier: CC0-1.0
#include "../Math/Polynomial_Sequence_Product.cpp"
#include <cassert>

int main()
{
    const auto result = sirkit::polynomial_sequence_product({{1, 1}, {1, 1}, {3}});
    assert((result == std::vector<std::uint32_t>{3, 6, 3}));
}
