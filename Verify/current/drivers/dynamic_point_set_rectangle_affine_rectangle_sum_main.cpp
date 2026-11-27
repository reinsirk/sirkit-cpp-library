// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    using Tree = PointRectangleAffineKDTree<>;
    struct Operation
    {
        Tree::Rectangle rectangle{};
        int type = 0, id = 0;
        std::uint32_t a = 0, b = 0;
    };

    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<Tree::Point> points;
    points.reserve(n + q);
    std::vector<std::uint32_t> weights(n);
    for (auto &weight : weights)
    {
        Tree::Point point{};
        verify_in >> point.x >> point.y >> weight;
        points.push_back(point);
    }
    std::vector<Operation> operations(q);
    for (auto &op : operations)
    {
        verify_in >> op.type;
        if (op.type == 0)
        {
            Tree::Point point{};
            verify_in >> point.x >> point.y >> op.a;
            op.id = static_cast<int>(points.size());
            points.push_back(point);
        }
        else if (op.type == 1)
        {
            verify_in >> op.id >> op.a;
        }
        else
        {
            auto &rect = op.rectangle;
            verify_in >> rect.left >> rect.bottom >> rect.right >> rect.top;
            if (op.type == 3)
            {
                verify_in >> op.a >> op.b;
            }
        }
    }

    Tree tree(points);
    for (int id = 0; id < n; ++id)
    {
        tree.set(id, weights[id]);
    }
    for (const auto &op : operations)
    {
        if (op.type <= 1)
        {
            tree.set(op.id, op.a);
        }
        else if (op.type == 2)
        {
            verify_out << tree.range_sum(op.rectangle) << '\n';
        }
        else
        {
            tree.range_affine(op.rectangle, op.a, op.b);
        }
    }
}
