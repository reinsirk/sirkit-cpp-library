// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    std::uint64_t n{};
    verify_in >> n;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const auto count = n * (n + 1) / 2;
    verify_out << *sirkit::kth_subarray_median(values, count / 2 + 1) << '\n';
}
