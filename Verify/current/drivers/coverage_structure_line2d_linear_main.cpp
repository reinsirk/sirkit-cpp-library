// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
int main() {
    int q{}; verify_in >> q;
    LineContainer2D points(std::int64_t{1} << 62);
    while (q--) {
        std::int64_t x{}, y{}, a{}, b{}; verify_in >> x >> y >> a >> b;
        points.add(x, y);
        verify_out << points.max_rounded_ll(a, b) << '\n';
    }
}
