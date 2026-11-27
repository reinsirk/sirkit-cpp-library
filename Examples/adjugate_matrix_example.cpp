// SPDX-License-Identifier: CC0-1.0
#include "../Math/Adjugate_Matrix.cpp"
#include <atcoder/modint>
#include <cassert>

int main()
{
    using Mint = atcoder::modint998244353;
    const sirkit::Matrix<Mint> a{{1, 2}, {3, 4}};
    const sirkit::Matrix<Mint> expected{{4, -2}, {-3, 1}};
    assert(sirkit::MatrixAdjugate(a) == expected);
}
