// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    int height{}, width{}, holes{};
    verify_in >> height >> width >> holes;
    const auto columns = static_cast<std::size_t>(width);
    std::vector<unsigned char> good(static_cast<std::size_t>(height) * columns, 1);
    for (int i = 0; i < holes; ++i)
    {
        std::size_t row{}, col{};
        verify_in >> row >> col;
        good[(row - 1) * columns + col - 1] = 0;
    }
    const auto result = sirkit::binary_grid_squares(height, width, [&](int row, int col)
    {
        return good[static_cast<std::size_t>(row) * columns + static_cast<std::size_t>(col)] != 0;
    });
    verify_out << result.count << '\n';
}
