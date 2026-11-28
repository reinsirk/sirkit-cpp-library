// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Static_Range_Minimum.cpp"
#include <cassert>

int main()
{
    const sirkit::StaticRangeMinimum<int> index({5, 2, 3, 2, 8});
    assert(index.argmin(1, 5) == 1);
    assert(index.min(2, 5) == 2);
}
