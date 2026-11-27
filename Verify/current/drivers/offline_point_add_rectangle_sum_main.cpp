// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<PointAdd> initial(n);
    for (auto &point : initial)
    {
        verify_in >> point.x >> point.y >> point.weight;
    }
    std::vector<PointAddRectangleSumOperation> operations;
    operations.reserve(q);
    while (q-- > 0)
    {
        int type = 0;
        verify_in >> type;
        if (type == 0)
        {
            PointAdd point{};
            verify_in >> point.x >> point.y >> point.weight;
            operations.emplace_back(point);
        }
        else
        {
            RectangleSumRequest rectangle{};
            verify_in >> rectangle.left >> rectangle.bottom >> rectangle.right >> rectangle.top;
            operations.emplace_back(rectangle);
        }
    }
    for (const auto answer : offline_point_add_rectangle_sum(initial, operations))
    {
        verify_out << answer << '\n';
    }
}
