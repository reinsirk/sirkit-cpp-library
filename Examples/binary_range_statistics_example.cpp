// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Binary_Range_Statistics.cpp"

#include <cassert>

int main()
{
    sirkit::BinaryRangeStatistics bits("001011");
    assert(bits.all_stats().longest[1] == 2);
    bits.flip(0, 2);
    assert(bits.stats(0, 4).longest[1] == 3);
    bits.assign(2, 6, false);
    assert(bits.all_stats().ones == 2);
}
