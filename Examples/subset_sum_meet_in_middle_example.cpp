// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Subset_Sum_Meet_In_Middle.cpp"
#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array<std::int64_t, 4> weights{5, -2, 5, 3};
    const sirkit::MeetInMiddleSubsetSum sums(weights);
    const auto best = sums.select_max_at_most(4);
    assert(best && best->sum == 3);
    assert(sums.count_exact(3) == 3);
    assert(sums.count_at_most(0) == 2);
}
