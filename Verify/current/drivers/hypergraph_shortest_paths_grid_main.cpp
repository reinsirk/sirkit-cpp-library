// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>
#include <string>
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
    int source{}, target{};
    const auto graph = [&]
    {
        std::array<std::vector<int>, 26> locations;
        for (int r = 0; r < h; ++r)
        {
            for (int c = 0; c < w; ++c)
            {
                const char cell = grid[r][c];
                const int v = r * w + c;
                if (cell == 'S')
                {
                    source = v;
                }
                else if (cell == 'G')
                {
                    target = v;
                }
                else if ('a' <= cell && cell <= 'z')
                {
                    locations[cell - 'a'].push_back(v);
                }
            }
        }
        std::array<sirkit::Hyperedge, 26> groups{};
        for (int i = 0; i < 26; ++i)
        {
            groups[i] = {locations[i]};
        }
        return sirkit::HypergraphShortestPaths(h * w, groups);
    }();
    const auto neighbors = [&](int v, auto visit)
    {
        const int r = v / w, c = v % w;
        if (r > 0 && grid[r - 1][c] != '#')
        {
            visit(v - w);
        }
        if (r + 1 < h && grid[r + 1][c] != '#')
        {
            visit(v + w);
        }
        if (c > 0 && grid[r][c - 1] != '#')
        {
            visit(v - 1);
        }
        if (c + 1 < w && grid[r][c + 1] != '#')
        {
            visit(v + 1);
        }
    };
    verify_out << graph.unit_distances(source, neighbors)[target] << '\n';
}
