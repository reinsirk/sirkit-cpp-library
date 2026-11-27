// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Linear_Add_Range_Min.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<std::int64_t> values{5, 4, 3, 2};
    sirkit::RangeLinearAddRangeMin data(values);
    data.range_add(1, 4, 2, -1);
    assert(data.range_min(2, 4) == 6);
    assert(!data.range_min(2, 2));
}
