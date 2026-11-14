// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

int main() {
    int n; std::cin >> n;
    std::vector<::sirkit::geo::PointLD> polygon(n);
    for (auto& p : polygon) std::cin >> p.x >> p.y;
    std::cout << std::fixed << std::setprecision(12);
    int q; std::cin >> q;
    while (q--) {
        ::sirkit::geo::PointLD p; std::cin >> p.x >> p.y;
        const auto where = ::sirkit::geo::contains(polygon, p);
        std::cout << (where == ::sirkit::geo::Location::Inside ? 2 : where == ::sirkit::geo::Location::Boundary ? 1 : 0) << '\n';
    }
}
