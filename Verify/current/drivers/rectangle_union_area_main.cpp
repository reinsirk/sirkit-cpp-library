// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<AreaRectangle> rectangles(n);
    for (auto &[left, bottom, right, top] : rectangles)
    {
        verify_in >> left >> bottom >> right >> top;
    }
    verify_out << rectangle_union_area(rectangles) << '\n';
}
