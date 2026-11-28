// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Monotone_Value_Shift.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<std::int64_t> initial{0, 2, 2, 5};
    sirkit::MonotoneValueShift sequence(initial);
    sequence.shift_between(0, 2, 2);
    assert(sequence.get(1) == 4);
    assert(sequence.count_between(4, 5) == 3);
    assert(sequence.lower_bound(4) == 1);
    assert(sequence.upper_bound(4) == 3);
    sequence.shift_between(4, 4, -2);
    const std::vector<std::int64_t> expected{2, 2, 2, 5};
    assert(sequence.values() == expected);
}
