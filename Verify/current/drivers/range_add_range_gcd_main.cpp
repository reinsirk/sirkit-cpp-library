// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    sirkit::RangeAddRangeGcd data(values);
    int q{};
    verify_in >> q;
    for (int i = 0; i < q; ++i)
    {
        int l{}, r{};
        std::int64_t delta{};
        verify_in >> delta >> l >> r;
        if (delta == 0)
        {
            verify_out << data.gcd(l - 1, r) << '\n';
        }
        else
        {
            data.add(l - 1, r, delta);
        }
    }
}
