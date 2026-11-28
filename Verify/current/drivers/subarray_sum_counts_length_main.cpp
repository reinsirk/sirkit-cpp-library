// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{}, maximum_length{};
    std::int64_t minimum_sum{};
    verify_in >> n >> maximum_length >> minimum_sum;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    verify_out << sirkit::count_subarrays_by_sum(values, {.minimum_sum = minimum_sum, .maximum_length = maximum_length}) << '\n';
}
