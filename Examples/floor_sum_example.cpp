// SPDX-License-Identifier: CC0-1.0
#include "../Math/Floor_Sum.cpp"
#include <cassert>

int main()
{
    assert(sirkit::floor_sum(4, 3, 2, 1) == 4);
    assert(sirkit::floor_sum(2, 3, -1, 0) == -1);
    assert(sirkit::min_of_mod_of_linear(4, 7, 2, 3) == 0);
}
