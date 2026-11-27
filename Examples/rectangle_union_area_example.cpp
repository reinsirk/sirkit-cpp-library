// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Rectangle_Union_Area.cpp"
#include <cassert>

int main()
{
    const std::vector<sirkit::AreaRectangle> rectangles{{0, 0, 2, 2}, {1, 1, 3, 3}};
    assert(sirkit::rectangle_union_area(rectangles) == 7);
}
