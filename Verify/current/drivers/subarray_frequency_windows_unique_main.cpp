// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const auto result = sirkit::analyze_frequency_bounded_subarrays(values, {.maximum_occurrences = 1});
    verify_out << result.length() << '\n';
}
