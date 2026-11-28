// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Persistent_Affine_Sequence.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array<long long, 5> initial{1, 2, 3, 4, 5};
    const sirkit::PersistentAffineSequence<long long> before(initial);
    const auto after_add = before.affine(1, 3, 1, 10);
    assert(after_add.sum(0, 3) == 26);
    assert(before.sum(0, 3) == 6);

    const auto copied = after_add.replace(2, 4, before.slice(0, 2));
    assert(copied.total() == 21);
    const auto reversed = copied.reverse(0, 4);
    assert(reversed.sum(0, 2) == 3);
}
