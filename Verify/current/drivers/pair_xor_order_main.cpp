// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n;
    std::uint64_t k;
    verify_in >> n >> k;
    std::vector<std::uint32_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    const sirkit::PairXorOrder order(std::move(values));
    verify_out << order.kth(k - 1) << '\n';
}
