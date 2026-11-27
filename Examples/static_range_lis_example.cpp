// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Static_Range_LIS.cpp"
#include <array>
#include <cassert>

int main()
{
    const sirkit::StaticRangeLIS lis(std::array{3, 1, 2, 2, 4});
    assert(lis.length(0, 5) == 3);
    assert(lis.length(1, 4) == 2);
    assert(lis.length(2, 4) == 1);
    assert(lis.length(2, 2) == 0);
}
