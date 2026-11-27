// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<int> data(n);
    for (auto &value : data)
    {
        verify_in >> value;
    }
    const StaticRangeMode<int> mode(data);
    while (q-- > 0)
    {
        int left = 0, right = 0;
        verify_in >> left >> right;
        const auto [value, count] = *mode.query(left, right);
        verify_out << value << ' ' << count << '\n';
    }
}
