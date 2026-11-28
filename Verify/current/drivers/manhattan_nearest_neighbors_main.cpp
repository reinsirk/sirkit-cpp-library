// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<sirkit::geo::ManhattanPoint> points(n);
    for (int i = 0; i < n; ++i)
    {
        points[i][0] = i + 1;
        verify_in >> points[i][1];
    }
    const auto nearest = sirkit::geo::manhattan_nearest_neighbors(points);
    for (int i = 0; i < n; ++i)
    {
        verify_out << nearest[i]->distance << (i + 1 == n ? '\n' : ' ');
    }
}
