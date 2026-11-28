// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<std::uint64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    std::uint64_t answer = 0;
    sirkit::for_each_subarray_aggregate_run(
        values, std::gcd<std::uint64_t, std::uint64_t>,
        [&](std::size_t, std::span<const sirkit::SubarrayAggregateRun<std::uint64_t>> runs)
        {
            // 正整数のGCDが1なら、区間を左へ伸ばしても1になる。
            if (runs.front().value == 1)
            {
                answer += runs.front().last;
            }
        });
    verify_out << answer << '\n';
}
