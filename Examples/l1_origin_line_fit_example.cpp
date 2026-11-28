// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/L1_Origin_Line_Fit.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <utility>

int main()
{
    const std::array<std::pair<std::int32_t, std::int32_t>, 3> points{{{4, 3}, {1, 1}, {-8, -6}}};
    const auto result = sirkit::l1_origin_line_fit(points);
    assert(result.cost == sirkit::Fraction(1, 4));
    assert(result.slope == sirkit::Fraction(3, 4) && !result.swapped);
}
