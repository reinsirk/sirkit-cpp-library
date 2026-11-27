// SPDX-License-Identifier: CC0-1.0
#include "../Math/Integer_Root.cpp"
#include <cassert>

int main()
{
    assert(sirkit::integer_kth_root(215, 3) == 5);
    assert(sirkit::integer_kth_root(216, 3) == 6);
}
