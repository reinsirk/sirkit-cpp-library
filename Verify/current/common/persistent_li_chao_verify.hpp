// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include <utility>
#include <vector>

template <bool Segments>
int VerifyPersistentLiChao()
{
    using T = std::int64_t;
    using Tree = sirkit::PersistentLiChaoTree<T>;
    struct Operation
    {
        Tree::Line line{};
        T first{};
        T last{};
        bool query = false;
    };
    const auto read_line = [](Operation &op)
    {
        if constexpr (Segments)
        {
            verify_in >> op.first >> op.last;
        }
        verify_in >> op.line.slope >> op.line.intercept;
    };

    int n, q;
    verify_in >> n >> q;
    std::vector<Operation> operations(static_cast<std::size_t>(n) + q);
    for (int i = 0; i < n; ++i)
    {
        read_line(operations[i]);
    }
    std::vector<T> coordinates;
    coordinates.reserve(q);
    for (std::size_t i = n; i < operations.size(); ++i)
    {
        int type;
        verify_in >> type;
        auto &op = operations[i];
        if (type == 0)
        {
            read_line(op);
        }
        else
        {
            op.query = true;
            verify_in >> op.first;
            coordinates.push_back(op.first);
        }
    }

    Tree tree(std::move(coordinates));
    Tree::Version current;
    for (const auto &op : operations)
    {
        if (!op.query)
        {
            if constexpr (Segments)
            {
                current = tree.add_segment(current, op.first, op.last, op.line);
            }
            else
            {
                current = tree.add_line(current, op.line);
            }
        }
        else if (const auto result = tree.get(current, op.first))
        {
            verify_out << *result << '\n';
        }
        else
        {
            verify_out << "INFINITY\n";
        }
    }
    return 0;
}
