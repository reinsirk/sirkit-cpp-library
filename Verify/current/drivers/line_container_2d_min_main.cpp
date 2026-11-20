// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>

int main() {
    int n, q;
    verify_in >> n >> q;
    // |a*x+b|<=2e18なので、既定の2^60より大きい上限を指定する。
    LineContainer2D points(std::int64_t{1} << 62);
    for (int i = 0; i < n; ++i) {
        std::int64_t a, b;
        verify_in >> a >> b;
        points.add(a, b);
    }
    for (int i = 0; i < q; ++i) {
        int type;
        verify_in >> type;
        if (type == 0) {
            std::int64_t a, b;
            verify_in >> a >> b;
            points.add(a, b);
        } else {
            std::int64_t x;
            verify_in >> x;
            verify_out << points.min_rounded_ll(x, 1) << '\n';
        }
    }
}
