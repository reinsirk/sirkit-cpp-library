// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Interval_Map.cpp"

#include <cassert>

int main()
{
    sirkit::IntervalMap<int> values(8, 0);
    values.assign(1, 4, 7);
    values.assign(4, 6, 7);
    const auto run = values.run_at(5);
    assert(run.left == 1 && run.right == 6 && run.value == 7);
    assert(values.get(6) == 0 && values.run_count() == 3);
}
