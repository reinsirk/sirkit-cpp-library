// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    const auto read_points = []
    {
        std::size_t n{};
        verify_in >> n;
        std::vector<sirkit::CountPointsInTriangle::Point> points(n);
        for (auto &[x, y] : points)
        {
            verify_in >> x >> y;
        }
        return points;
    };
    const auto vertices = read_points();
    const auto points = read_points();
    const sirkit::CountPointsInTriangle triangles(vertices, points);

    std::size_t q{};
    verify_in >> q;
    while (q-- != 0)
    {
        std::size_t a{}, b{}, c{};
        verify_in >> a >> b >> c;
        verify_out << triangles.count(a, b, c) << '\n';
    }
}
