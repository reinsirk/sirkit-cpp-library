// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    std::size_t n{};
    int maximum{}, minimum{};
    verify_in >> n >> maximum >> minimum;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    verify_out << sirkit::analyze_subarrays_with_extrema(values, minimum, maximum).count << '\n';
}
