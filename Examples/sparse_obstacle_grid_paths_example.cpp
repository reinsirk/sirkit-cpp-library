// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Sparse_Obstacle_Grid_Paths.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <utility>

int main()
{
    const auto choose = [](std::int64_t n, std::int64_t k)
    {
        std::int64_t result = 1;
        for (std::int64_t i = 1; i <= k; ++i)
        {
            result = result * (n - i + 1) / i;
        }
        return result;
    };
    const std::array walls{std::pair{1, 1}};
    const auto paths = sirkit::count_sparse_grid_paths(3, 3, walls, choose);
    assert(paths == 2);
}
