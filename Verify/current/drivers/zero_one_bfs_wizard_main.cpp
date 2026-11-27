// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdlib>
#include <string>
#include <vector>

int main()
{
    int h, w, sy, sx, ty, tx;
    verify_in >> h >> w >> sy >> sx >> ty >> tx;
    std::vector<std::string> grid(h);
    for (auto &row : grid)
    {
        verify_in >> row;
    }
    const auto result = zero_one_bfs(h * w, (sy - 1) * w + sx - 1, [&](int v, auto visit)
                                     {
                                         const int y = v / w, x = v % w;
                                         for (int dy = -2; dy <= 2; ++dy)
                                         {
                                             for (int dx = -2; dx <= 2; ++dx)
                                             {
                                                 const int ny = y + dy, nx = x + dx;
                                                 if (ny >= 0 && ny < h && nx >= 0 && nx < w && grid[ny][nx] == '.')
                                                 {
                                                     visit(ny * w + nx, std::abs(dy) + std::abs(dx) == 1 ? 0 : 1);
                                                 }
                                             }
                                         }
                                     });
    verify_out << result.distance[(ty - 1) * w + tx - 1] << '\n';
}
