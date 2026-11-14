// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <vector>

int main() {
    int n{};
    verify_in >> n;
    std::vector<::sirkit::geo::PointI> polygon(n);
    for (auto& p : polygon) verify_in >> p.x >> p.y;
    ::sirkit::geo::PointI p;
    verify_in >> p.x >> p.y;
    verify_out << (::sirkit::geo::contains(polygon, p) == ::sirkit::geo::Location::Inside ? "INSIDE" : "OUTSIDE") << '\n';
}
