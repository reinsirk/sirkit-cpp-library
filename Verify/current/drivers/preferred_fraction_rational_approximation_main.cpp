// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>

// Stern–Brocotの両端との差をFractionで保持し、移動のたびに差分更新する。
int main() {
    int cases{}; verify_in >> cases;
    while (cases--) {
        std::int64_t limit{}, x{}, y{}; verify_in >> limit >> x >> y;
        const Fraction target(x, y);
        if (target.numerator() <= limit && target.denominator() <= limit) {
            verify_out << target.numerator() << ' ' << target.denominator() << ' '
                       << target.numerator() << ' ' << target.denominator() << '\n';
            continue;
        }
        std::int64_t a = 0, b = 1, c = 1, d = 0;
        Fraction left_gap = target, right_gap(1);
        while (a + c <= limit && b + d <= limit) {
            if (left_gap > right_gap) {
                auto step = (left_gap / right_gap).floor();
                auto cap = (limit - a) / c;
                if (d) cap = std::min(cap, (limit - b) / d);
                if (step >= cap) {
                    a += cap * c; b += cap * d;
                    break;
                }
                a += step * c; b += step * d;
                left_gap += -(right_gap * Fraction(step));
            } else {
                auto step = (right_gap / left_gap).floor();
                auto cap = (limit - d) / b;
                if (a) cap = std::min(cap, (limit - c) / a);
                if (step >= cap) {
                    c += cap * a; d += cap * b;
                    break;
                }
                c += step * a; d += step * b;
                right_gap -= left_gap * Fraction(step);
            }
        }
        verify_out << a << ' ' << b << ' ' << c << ' ' << d << '\n';
    }
}
