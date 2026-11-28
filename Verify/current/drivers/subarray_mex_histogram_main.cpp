// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    for (const auto count : sirkit::subarray_mex_histogram(values))
    {
        verify_out << count << '\n';
    }
}
