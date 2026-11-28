// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cmath>

int main()
{
    int tests = 0;
    verify_in >> tests;
    verify_out.fixed(12);
    while (tests-- > 0)
    {
        int x_a = 0, y_a = 0, radius_a = 0, x_b = 0, y_b = 0, radius_b = 0;
        verify_in >> x_a >> y_a >> radius_a >> x_b >> y_b >> radius_b;
        const sirkit::geo::Circle a{{static_cast<long double>(x_a), static_cast<long double>(y_a)},
                                    static_cast<long double>(radius_a)};
        const sirkit::geo::Circle b{{static_cast<long double>(x_b), static_cast<long double>(y_b)},
                                    static_cast<long double>(radius_b)};
        const auto tangents = sirkit::geo::circle_tangents(a, b);
        long double sum = 0;
        for (const auto &line : *tangents)
        {
            const auto x = line.normal.x, y = line.normal.y;
            const auto c = -line.normal.dot(line.first);
            sum += std::abs(x + y + c) / std::max({std::abs(x), std::abs(y), std::abs(c)});
        }
        verify_out << sum << '\n';
    }
}
