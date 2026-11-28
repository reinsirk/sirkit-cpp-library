// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

int main()
{
    int h{}, w{};
    verify_in >> h >> w;
    std::vector<int> floors(h * w);
    for (int &height : floors)
    {
        verify_in >> height;
    }
    std::vector<sirkit::SpanningTreeEdge> edges;
    edges.reserve(2 * h * w);
    for (int row = 0; row < h; ++row)
    {
        for (int column = 0; column < w; ++column)
        {
            const int v = row * w + column;
            if (row + 1 < h)
            {
                edges.push_back({v, v + w, std::min(floors[v], floors[v + w])});
            }
            if (column + 1 < w)
            {
                edges.push_back({v, v + 1, std::min(floors[v], floors[v + 1])});
            }
        }
    }
    const sirkit::KruskalReconstructionTree forest(h * w, edges, sirkit::KruskalOrder::decreasing);
    int q{};
    verify_in >> q;
    while (q-- > 0)
    {
        int a{}, b{}, y{}, c{}, d{}, z{};
        verify_in >> a >> b >> y >> c >> d >> z;
        const int from = (a - 1) * w + b - 1;
        const int to = (c - 1) * w + d - 1;
        const auto lower = std::min(y, z);
        const auto maximum_floor = forest.merge_weight(from, to).value_or(lower);
        verify_out << (y + z - 2 * std::min<std::int64_t>(lower, maximum_floor)) << '\n';
    }
}
