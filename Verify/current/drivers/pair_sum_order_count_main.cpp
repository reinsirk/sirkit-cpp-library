// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    std::int64_t x{};
    verify_in >> n >> x;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const sirkit::PairSumOrder order(values, values);
    verify_out << order.count_equal(x) << '\n';
}
