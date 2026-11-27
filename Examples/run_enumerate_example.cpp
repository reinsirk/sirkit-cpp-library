// SPDX-License-Identifier: CC0-1.0
#include "../String/Run_Enumerate.cpp"
#include <cassert>

int main()
{
    const auto runs = sirkit::run_enumerate("abababa");
    assert(runs.size() == 1);
    assert((runs[0] == sirkit::StringRun{2, 0, 7}));
}
