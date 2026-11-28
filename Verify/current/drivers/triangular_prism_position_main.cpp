// SPDX-License-Identifier: CC0-1.0
#include <array>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    std::array<Point3i, 4> points{};
    for (auto &[x, y, z] : points)
    {
        verify_in >> x >> y >> z;
    }
    const auto position = triangular_prism_position(points[0], points[1], points[2], points[3]);
    verify_out << (position == PrismPosition::kInside ? "YES\n" : "NO\n");
}
