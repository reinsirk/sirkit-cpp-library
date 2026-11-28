// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{}, width{};
    verify_in >> n >> width;
    std::vector<std::uint32_t> lower(n), upper(n);
    for (auto &value : lower)
    {
        verify_in >> value;
    }
    for (auto &value : upper)
    {
        verify_in >> value;
    }
    verify_out << sirkit::count_bounded_nondecreasing_sequences(lower, upper) << '\n';
}
