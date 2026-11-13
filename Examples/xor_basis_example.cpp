// SPDX-License-Identifier: CC0-1.0
#include "../Math/Xor_Basis.cpp"
#include <cassert>
#include <cstdint>
#include <iostream>
int main() {
    XorBasis basis;
    assert(basis.insert(3) && basis.insert(5));
    std::cout << basis.max_xor() << '\n'; // 6
    std::cout << *basis.kth(2) << '\n';   // 5
    assert(basis.rank() == 2 && basis.contains(6) && !basis.contains(1));
    assert(basis.contains(0) && !basis.contains_nonempty(0));
    assert(basis.min_xor(7) == 1 && basis.max_xor(7) == 7);
    assert(!basis.kth(4));
    XorBasis other;
    other.insert(6); basis.merge(other);
    assert(basis.has_nonempty_zero() && basis.contains_nonempty(0));
    basis.clear(); basis.swap(other);
    assert(basis.rank() == 1 && other.rank() == 0);
}
