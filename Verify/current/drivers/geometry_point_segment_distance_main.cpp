// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>

int main() {
    ::sirkit::geo::PointI point;
    ::sirkit::geo::Segment<std::int64_t> segment;
    verify_in >> point.x >> point.y;
    verify_in >> segment.a.x >> segment.a.y >> segment.b.x >> segment.b.y;
    verify_out.fixed(12);
    verify_out << ::sirkit::geo::distance(point, segment) << '\n';
}
