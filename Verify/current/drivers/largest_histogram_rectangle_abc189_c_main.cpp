// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::uint64_t> heights(n);
    for (auto &height : heights)
    {
        verify_in >> height;
    }
    verify_out << sirkit::largest_histogram_rectangle(heights).area << '\n';
}
