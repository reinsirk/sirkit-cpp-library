// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<sirkit::PolygonChordMoments::Point> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    const sirkit::PolygonChordMoments polygon(points);
    verify_out.fixed(12);
    for (int i = 0; i < q; ++i)
    {
        int u = 0, v = 0;
        verify_in >> u >> v;
        const auto [x, y] = *polygon.slice(u - 1, v - 1).centroid;
        verify_out << x << ' ' << y << '\n';
    }
}
