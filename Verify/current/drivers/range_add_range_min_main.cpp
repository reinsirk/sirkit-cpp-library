// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto &x : values)
    {
        verify_in >> x;
    }
    sirkit::RangeAddRangeMin<std::int64_t> minimum(values);
    for (int i = 0; i < q; ++i)
    {
        int type = 0, left = 0, right = 0;
        verify_in >> type >> left >> right;
        if (type == 0)
        {
            std::int64_t delta = 0;
            verify_in >> delta;
            minimum.add(left, right, delta);
        }
        else
        {
            verify_out << *minimum.minimum(left, right) << '\n';
        }
    }
}
