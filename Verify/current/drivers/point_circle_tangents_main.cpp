// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

int main()
{
    long double radius = 0, distance = 0;
    verify_in >> radius >> distance;
    const sirkit::geo::RealPoint point{distance, 0};
    const auto tangents = sirkit::geo::point_circle_tangents({{0, 0}, radius}, point, {0, 0});
    verify_out.fixed(12) << (point - tangents.front().first).norm() << '\n';
}
