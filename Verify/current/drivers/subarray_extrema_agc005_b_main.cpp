// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::uint64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const auto sums = sirkit::subarray_extremum_sums_by_length<std::uint64_t>(values);
    verify_out << std::accumulate(sums.begin(), sums.end(), std::uint64_t{0}) << '\n';
}
