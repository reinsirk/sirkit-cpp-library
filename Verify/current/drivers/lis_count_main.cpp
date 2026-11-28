// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    verify_out << sirkit::count_lis(values).count << '\n';
}
