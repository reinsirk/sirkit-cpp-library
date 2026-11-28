// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Lattice_Shape_Transforms.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <utility>

int main()
{
    using Point = std::pair<std::int64_t, std::int64_t>;
    const std::array source{Point{0, 0}, Point{1, 0}, Point{0, 1}, Point{0, 2}};
    const std::array target{Point{5, 3}, Point{5, 4}, Point{4, 3}, Point{3, 3}};
    const auto matches = sirkit::lattice_shape_transforms(source, target);
    assert(matches.size() == 1 && matches[0] == sirkit::LatticeOrientation::kRotate90);
}
