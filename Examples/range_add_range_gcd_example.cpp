// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Add_Range_Gcd.cpp"
#include <cassert>
#include <vector>

int main()
{
    const std::vector<long long> values{6, 9, 15};
    sirkit::RangeAddRangeGcd data(values);
    data.add(1, 3, 3);
    assert(data.gcd(0, 3) == 6);
    assert(data.get(2) == 18);
}
