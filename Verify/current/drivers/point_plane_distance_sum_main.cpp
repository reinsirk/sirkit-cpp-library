// SPDX-License-Identifier: CC0-1.0
#include <cmath>
#include <vector>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int n = 0;
    verify_in >> n;
    constexpr double kScale = 1000000;
    const auto read_point = []
    {
        double x = 0, y = 0, z = 0;
        verify_in >> x >> y >> z;
        return Point3i{std::llround(x * kScale), std::llround(y * kScale), std::llround(z * kScale)};
    };
    const auto p = read_point();
    std::vector<Point3i> points(n);
    for (auto &point : points)
    {
        point = read_point();
    }
    double total = 0, rounding = 0;
    for (int k = 2; k < n; ++k)
    {
        for (int j = 1; j < k; ++j)
        {
            for (int i = 0; i < j; ++i)
            {
                const auto term = std::abs(*signed_plane_distance(points[i], points[j], points[k], p)) - rounding;
                const auto sum = total + term;
                rounding = (sum - total) - term;
                total = sum;
            }
        }
    }
    verify_out.fixed(12) << total / kScale << '\n';
}
