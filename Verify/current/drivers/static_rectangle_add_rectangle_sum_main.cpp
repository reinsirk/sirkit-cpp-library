// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<StaticRectangleAdd> additions(n);
    std::vector<StaticRectangleSumQuery> queries(q);
    for (auto &rectangle : additions)
    {
        verify_in >> rectangle.left >> rectangle.bottom >> rectangle.right >> rectangle.top >> rectangle.weight;
    }
    for (auto &rectangle : queries)
    {
        verify_in >> rectangle.left >> rectangle.bottom >> rectangle.right >> rectangle.top;
    }
    for (const auto answer : static_rectangle_add_rectangle_sum(additions, queries))
    {
        verify_out << answer << '\n';
    }
}
