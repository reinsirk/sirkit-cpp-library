// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::PalindromicPathEdge> edges(m);
    for (auto &[from, to, label] : edges)
    {
        char ch{};
        verify_in >> from >> to >> ch;
        --from;
        --to;
        label = static_cast<std::uint8_t>(ch);
    }
    const sirkit::PalindromicShortestPaths paths(n, edges, true);
    verify_out << paths.distance(0, n - 1) << '\n';
}
