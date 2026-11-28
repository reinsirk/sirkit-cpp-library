// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Subarray_Aggregate_Runs.cpp"

#include <array>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <span>

int main()
{
    constexpr std::array values{6, 10, 15};
    std::size_t count = 0;
    sirkit::for_each_subarray_aggregate_run(
        values, std::gcd<int, int>,
        [&](std::size_t, std::span<const sirkit::SubarrayAggregateRun<int>> runs)
        {
            for (const auto &run : runs)
            {
                if (run.value == 1)
                {
                    count += run.last - run.first;
                }
            }
        });
    std::cout << count << '\n'; // 1
}
