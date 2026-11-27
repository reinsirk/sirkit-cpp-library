// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    PointSetRangeFrequency<int> frequency(values);
    while (q-- > 0)
    {
        int type = 0, first = 0, second = 0;
        verify_in >> type >> first >> second;
        if (type == 0)
        {
            frequency.set(first, second);
        }
        else
        {
            int value = 0;
            verify_in >> value;
            verify_out << frequency.count(first, second, value) << '\n';
        }
    }
}
