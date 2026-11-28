// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Interval_Scheduling.cpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <vector>

int main()
{
    const std::array intervals{
        sirkit::ScheduleInterval{0, 2, 3},
        sirkit::ScheduleInterval{1, 4, 10},
        sirkit::ScheduleInterval{2, 4, 4},
    };
    const auto count = sirkit::interval_schedule_max_count(intervals);
    assert((count == std::vector<std::size_t>{0, 2}));
    const auto best = sirkit::interval_schedule_max_weight(intervals);
    assert(best.weight == 10 && best.indices == std::vector<std::size_t>{1});
}
