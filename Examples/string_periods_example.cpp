// SPDX-License-Identifier: CC0-1.0
#include "../String/String_Periods.cpp"
#include <cassert>
#include <cstddef>
#include <vector>

int main()
{
    const sirkit::StringPeriods index("abababa");
    assert(index.least_period(index.size()) == 2);
    assert(index.primitive_root_length(index.size()) == 7);
    assert(index.primitive_root_length(6) == 2);
    assert((index.periods(index.size()) == std::vector<std::size_t>{2, 4, 6, 7}));
    assert((index.proper_borders(index.size()) == std::vector<std::size_t>{5, 3, 1}));
}
