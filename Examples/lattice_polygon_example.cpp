// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Lattice_Polygon.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <utility>

int main()
{
    using Point = std::pair<std::int64_t, std::int64_t>;
    const std::array triangle{Point{0, 0}, Point{4, 0}, Point{2, 3}};
    const auto counts = sirkit::lattice_polygon_points(triangle);
    assert(counts.interior == 4);
    assert(counts.boundary == 6);
    assert(counts.total() == 10);
}
