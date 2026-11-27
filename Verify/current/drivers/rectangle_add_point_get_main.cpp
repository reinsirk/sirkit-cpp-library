// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    const auto read_rectangle = []
    {
        RectangleAdd rectangle{};
        verify_in >> rectangle.left >> rectangle.bottom >> rectangle.right >> rectangle.top >> rectangle.weight;
        return rectangle;
    };
    std::vector<RectangleAdd> initial(n);
    for (auto &rectangle : initial)
    {
        rectangle = read_rectangle();
    }
    std::vector<RectangleAddPointGetOperation> operations;
    operations.reserve(q);
    for (int i = 0; i < q; ++i)
    {
        int type = 0;
        verify_in >> type;
        if (type == 0)
        {
            operations.emplace_back(read_rectangle());
        }
        else
        {
            RectanglePointGet point{};
            verify_in >> point.x >> point.y;
            operations.emplace_back(point);
        }
    }
    for (auto answer : offline_rectangle_add_point_get(initial, operations))
    {
        verify_out << answer << '\n';
    }
}
