// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Nondecreasing_Subsequence.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array symbols{2, 0, 1, 1, 2};
    sirkit::RangeNondecreasingSubsequence<3> range(symbols);
    assert(range.longest(0, 5) == 4);
    assert(range.longest(0, 5, 1, 2) == 3);
    assert(range.replacement_count(0, 5) == 1);
    range.set(0, 0);
    assert(range.all() == 5);
}
