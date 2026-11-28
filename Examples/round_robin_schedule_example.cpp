// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Round_Robin_Schedule.cpp"

#include <iostream>

int main()
{
    const sirkit::RoundRobinSchedule schedule(4);
    std::cout << schedule.round_count() << '\n';  // 3
    std::cout << schedule.partner(0, 0) << '\n';  // 3
    std::cout << schedule.round_of(1, 2) << '\n'; // 0
}
