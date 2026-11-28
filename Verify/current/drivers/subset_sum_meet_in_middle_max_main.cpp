// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t count = 0;
    std::int64_t limit = 0;
    verify_in >> count >> limit;
    std::vector<std::int64_t> weights(count);
    for (auto &weight : weights)
    {
        verify_in >> weight;
    }
    const sirkit::MeetInMiddleSubsetSum sums(weights);
    verify_out << sums.select_max_at_most(limit)->sum << '\n';
}
