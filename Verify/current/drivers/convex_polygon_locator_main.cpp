// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n;
    verify_in >> n;
    using Point = sirkit::geo::Point<std::int64_t, std::int64_t>;
    std::vector<Point> vertices(n);
    for (auto &point : vertices)
    {
        verify_in >> point.x >> point.y;
    }
    const sirkit::geo::ConvexPolygonLocator locator(std::move(vertices));
    int q;
    verify_in >> q;
    while (q-- > 0)
    {
        Point point;
        verify_in >> point.x >> point.y;
        switch (locator.locate(point))
        {
        case sirkit::geo::Location::Inside:
            verify_out << "IN\n";
            break;
        case sirkit::geo::Location::Boundary:
            verify_out << "ON\n";
            break;
        case sirkit::geo::Location::Outside:
            verify_out << "OUT\n";
            break;
        }
    }
}
