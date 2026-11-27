// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <string>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<MinimumEnclosingCircle::Point> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    const auto circle = minimum_enclosing_circle(points);
    std::string boundary;
    boundary.reserve(points.size());
    for (const auto &point : points)
    {
        boundary.push_back(circle->on_boundary(point) ? '1' : '0');
    }
    verify_out << boundary << '\n';
}
