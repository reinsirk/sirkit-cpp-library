// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>

int main()
{
    int queries;
    verify_in >> queries;
    sirkit::DynamicWaveletMatrix<std::uint64_t, 60> values;
    while (queries-- > 0)
    {
        int type;
        std::uint64_t value;
        verify_in >> type >> value;
        if (type == 1)
        {
            values.insert(values.size(), value);
        }
        else
        {
            std::size_t k;
            verify_in >> k;
            const std::size_t n = values.size();
            const std::size_t rank = values.count_less(0, n, value + (type == 2));
            if ((type == 2 ? rank : n - rank) < k)
            {
                verify_out << -1 << '\n';
            }
            else
            {
                verify_out << values.kth_smallest(0, n, type == 2 ? rank - k : rank + k - 1) << '\n';
            }
        }
    }
}
