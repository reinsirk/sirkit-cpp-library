// SPDX-License-Identifier: CC0-1.0
#include "../String/Lyndon_Factorization.cpp"
#include <cassert>
#include <cstddef>
#include <vector>

int main()
{
    const auto cuts = sirkit::lyndon_factorization("baba");
    assert((cuts == std::vector<std::size_t>{0, 1, 3, 4}));
}
