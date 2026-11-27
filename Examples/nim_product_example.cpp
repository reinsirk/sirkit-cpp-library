// SPDX-License-Identifier: CC0-1.0
#include "../Math/Nim_Product.cpp"
#include <cassert>

int main()
{
    assert(sirkit::nim_product(2, 2) == 3);
    assert(sirkit::nim_product(2, 3) == 1);
}
