// SPDX-License-Identifier: CC0-1.0
#include "../String/Run_Length_Z.cpp"
#include <array>
#include <cassert>

int main()
{
    using Run = sirkit::RunLengthZ::Run;
    const sirkit::RunLengthZ index(std::array{Run{'a', 2}, Run{'b', 1}, Run{'a', 2}});
    assert(index.size() == 5);
    assert(index.prefix_at(3) == 2);
    assert(index.sum_prefix_lengths() == 9);
}
