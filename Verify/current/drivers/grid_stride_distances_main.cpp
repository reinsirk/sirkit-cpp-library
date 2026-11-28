// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <string>
#include <vector>

int main()
{
    int h{}, w{}, x1{}, y1{}, x2{}, y2{};
    std::uint64_t k{};
    verify_in >> h >> w >> k >> x1 >> y1 >> x2 >> y2;
    static_cast<void>(w);
    std::vector<std::string> grid(h);
    for (auto &row : grid)
    {
        verify_in >> row;
    }
    verify_out << sirkit::grid_stride_distance(grid, {x1 - 1, y1 - 1}, {x2 - 1, y2 - 1}, k, '@') << '\n';
}
