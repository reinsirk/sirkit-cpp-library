// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>
#include <string>
#include <utility>
#include <vector>

int main()
{
    int h{}, w{};
    verify_in >> h >> w;
    std::vector<std::string> grid(h);
    for (auto &row : grid)
    {
        verify_in >> row;
    }
    const int source = h * w;
    const int target = source + 1;
    sirkit::LowerBoundFlow<int> graph(source + 2);
    constexpr std::array steps{std::pair{0, -1}, std::pair{0, 1}, std::pair{-1, 0}, std::pair{1, 0}};
    for (int i = 0; i < h; ++i)
    {
        for (int j = 0; j < w; ++j)
        {
            if (grid[i][j] == '1')
            {
                continue;
            }
            const int v = i * w + j;
            const int lower = grid[i][j] == '2';
            if ((i + j) % 2 == 0)
            {
                graph.add_edge(source, v, lower, 1);
                for (const auto &[di, dj] : steps)
                {
                    const int row = i + di;
                    const int column = j + dj;
                    if (0 <= row && row < h && 0 <= column && column < w && grid[row][column] != '1')
                    {
                        graph.add_edge(v, row * w + column, 0, 1);
                    }
                }
            }
            else
            {
                graph.add_edge(v, target, lower, 1);
            }
        }
    }
    verify_out << (graph.min_flow(source, target, source / 2) ? "Yes\n" : "No\n");
}
