// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    verify_in >> n;
    std::vector<std::int64_t> positions(n);
    for (auto &position : positions)
    {
        verify_in >> position;
    }
    verify_out << sirkit::line_chip_firing(positions).operations << '\n';
}
