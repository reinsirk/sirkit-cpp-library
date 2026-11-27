// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    RangeLinearAddRangeMin data(values);
    while (q-- > 0)
    {
        int type = 0, left = 0, right = 0;
        verify_in >> type >> left >> right;
        if (type == 0)
        {
            std::int64_t b = 0, c = 0;
            verify_in >> b >> c;
            data.range_add(left, right, b, c);
        }
        else
        {
            verify_out << *data.range_min(left, right) << '\n';
        }
    }
}
