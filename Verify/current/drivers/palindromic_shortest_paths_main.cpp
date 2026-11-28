// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <string>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<sirkit::PalindromicPathEdge> edges;
    for (int from = 0; from < n; ++from)
    {
        std::string row;
        verify_in >> row;
        for (int to = 0; to < n; ++to)
        {
            if (row[to] != '-')
            {
                edges.push_back({from, to, static_cast<std::uint8_t>(row[to])});
            }
        }
    }
    const sirkit::PalindromicShortestPaths paths(n, edges);
    for (int from = 0; from < n; ++from)
    {
        for (int to = 0; to < n; ++to)
        {
            verify_out << paths.distance(from, to) << (to + 1 == n ? '\n' : ' ');
        }
    }
}
