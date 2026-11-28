// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <vector>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    std::int64_t x = 0;
    int q = 0;
    verify_in >> x >> q;
    std::vector<std::int64_t> values(2 * q + 1);
    values[0] = x;
    for (int i = 1; i <= 2 * q; ++i)
    {
        verify_in >> values[i];
    }
    WeightedL1Statistics statistics(values);
    statistics.insert(x);
    for (int i = 1; i <= q; ++i)
    {
        statistics.insert(values[2 * i - 1]);
        statistics.insert(values[2 * i]);
        verify_out << *statistics.kth(i) << '\n';
    }
}
