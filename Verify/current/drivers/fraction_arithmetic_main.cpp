// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>

int main() {
    std::int64_t n = 0, k = 0;
    verify_in >> n >> k;
    const Fraction lower = Fraction(k - 1) / Fraction(n);
    const Fraction equal = Fraction(1) / Fraction(n);
    const Fraction upper = Fraction(1) - lower - equal;
    const Fraction answer = equal * equal * equal
        + Fraction(3) * equal * equal * (lower + upper)
        + Fraction(6) * lower * equal * upper;
    verify_out.fixed(18);
    verify_out << static_cast<long double>(answer) << '\n';
}
