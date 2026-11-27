// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Point_Set_Range_Frequency.cpp"

#include <cassert>
#include <vector>

int main()
{
    const std::vector<int> values{4, 7, 4, 2};
    sirkit::PointSetRangeFrequency<int> frequency(values);
    assert(frequency.count(1, 4, 4) == 1);
    frequency.set(3, 4);
    assert(frequency.count(1, 4, 4) == 2);
    assert(frequency.get(3) == 4);
}
