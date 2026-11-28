// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>

int main()
{
    std::size_t height{}, width{};
    verify_in >> height >> width;
    const auto answer = sirkit::largest_grid_rectangle(height, width, [](std::size_t, std::size_t)
                                                       {
                                                           int cell{};
                                                           verify_in >> cell;
                                                           return cell == 0;
                                                       });
    verify_out << answer.area << '\n';
}
