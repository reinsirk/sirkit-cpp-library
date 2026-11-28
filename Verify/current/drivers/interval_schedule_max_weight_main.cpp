// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    std::size_t n = 0, slots = 0;
    verify_in >> n >> slots;
    std::vector<sirkit::ScheduleInterval> intervals(n);
    for (auto &interval : intervals)
    {
        verify_in >> interval.left >> interval.right >> interval.weight;
        ++interval.right;
    }
    verify_out << sirkit::interval_schedule_max_weight(intervals).weight << '\n';
}
