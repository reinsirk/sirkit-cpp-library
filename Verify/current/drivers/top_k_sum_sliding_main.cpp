// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    std::size_t n, m, k;
    verify_in >> n >> m >> k;
    std::vector<int> values(n);
    sirkit::TopKSum<int, false, std::int64_t> least(k);
    for (std::size_t i = 0; i < n; ++i)
    {
        verify_in >> values[i];
        least.insert(values[i]);
        if (i >= m)
        {
            least.erase(values[i - m]);
            verify_out << ' ';
        }
        if (i + 1 >= m)
        {
            verify_out << least.sum();
        }
    }
    verify_out << '\n';
}
