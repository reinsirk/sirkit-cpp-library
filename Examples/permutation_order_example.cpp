// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Permutation_Order.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<int> permutation{2, 0, 3, 1};
    const auto rank = sirkit::permutation_rank(permutation);
    assert(rank == 13 && sirkit::permutation_unrank(4, *rank) == permutation);
    const auto code = sirkit::permutation_lehmer_code(permutation);
    assert((code == std::vector<int>{2, 0, 1, 0}));
    assert(sirkit::permutation_from_lehmer_code(code) == permutation);
}
