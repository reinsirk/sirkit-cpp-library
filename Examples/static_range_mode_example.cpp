// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Static_Range_Mode.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<int> a{9, 7, 1, 7, 2};
    const sirkit::StaticRangeMode<int> mode(a);
    const auto [value, count] = *mode.query(0, 5);
    assert(value == 7 && count == 2);
    assert(!mode.query(2, 2));
}
