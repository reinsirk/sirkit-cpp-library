// SPDX-License-Identifier: CC0-1.0
#include "../Math/Sparse_Matrix_Determinant.cpp"
#include <array>
#include <cassert>

int main()
{
    const std::array entries{
        sirkit::SparseDeterminantEntry{0, 0, 2},
        sirkit::SparseDeterminantEntry{0, 1, 7},
        sirkit::SparseDeterminantEntry{1, 1, 3},
    };
    assert(sirkit::sparse_determinant_mod998244353(2, entries) == 6);
}
