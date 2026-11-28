// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    std::size_t n, k, q;
    verify_in >> n >> k >> q;
    sirkit::TopKSum<int, true, std::int64_t> greatest(k);
    std::vector<int> values(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        greatest.insert(0);
    }
    for (std::size_t i = 0; i < q; ++i)
    {
        std::size_t position;
        int value;
        verify_in >> position >> value;
        --position;
        greatest.erase(values[position]);
        values[position] = value;
        greatest.insert(value);
        verify_out << greatest.sum() << '\n';
    }
}
