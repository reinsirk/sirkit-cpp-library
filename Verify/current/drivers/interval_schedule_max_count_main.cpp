// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<sirkit::ScheduleInterval> intervals(n);
    for (auto &interval : intervals)
    {
        verify_in >> interval.left >> interval.right;
    }
    verify_out << sirkit::interval_schedule_max_count(intervals).size() << '\n';
}
