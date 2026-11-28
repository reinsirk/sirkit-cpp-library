// SPDX-License-Identifier: CC0-1.0
#include <vector>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int n = 0;
    verify_in >> n;
    std::vector<TrapezoidPoint> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    verify_out << count_trapezoids(points).trapezoids << '\n';
}
