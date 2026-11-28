// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Equal_Runs.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array values{1, 1, 2, 2, 2, 3};
    sirkit::RangeEqualRuns<int> runs(values);
    const auto before = runs.query(0, 6);
    assert(before.prefix == 2 && before.suffix == 1 && before.longest == 3);
    runs.set(5, 2);
    assert(runs.all().longest == 4);
}
