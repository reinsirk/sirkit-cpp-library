// SPDX-License-Identifier: CC0-1.0
#include "../Math/Primitive_Root.cpp"
#include <cassert>

int main()
{
    const sirkit::PrimeMultiplicativeGroup group(17);
    assert(group.primitive_root() == 3);
    assert(group.order(4) == 4);
    assert(!group.order(17));
    assert(group.is_generator(3));
}
