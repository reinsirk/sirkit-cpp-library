// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    std::int64_t target{};
    verify_in >> n >> target;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    verify_out << sirkit::count_subarrays_by_sum(values, {.minimum_sum = target, .maximum_sum = target}) << '\n';
}
