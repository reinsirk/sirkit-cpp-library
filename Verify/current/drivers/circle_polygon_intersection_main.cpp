// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n = 0, radius = 0;
    verify_in >> n >> radius;
    std::vector<std::pair<double, double>> vertices(n);
    for (auto &[x, y] : vertices)
    {
        int px = 0, py = 0;
        verify_in >> px >> py;
        x = px;
        y = py;
    }
    verify_out.fixed(12) << sirkit::circle_polygon_intersection_area(vertices, radius) << '\n';
}
