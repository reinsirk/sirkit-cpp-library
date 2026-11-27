// SPDX-License-Identifier: CC0-1.0
#include "../Math/Newton_Basis.cpp"
#include <cassert>

int main()
{
    const std::vector<std::uint32_t> f{1, 2, 1}, points{1, 2};
    const auto b = sirkit::monomial_to_newton_basis(f, points);
    assert((b == std::vector<std::uint32_t>{4, 5, 1}));
    assert(sirkit::newton_to_monomial_basis(b, points) == f);
}
