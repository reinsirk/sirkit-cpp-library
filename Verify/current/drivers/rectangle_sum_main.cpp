// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>

int main()
{
    int n{}, q{};
    verify_in >> n >> q;

    std::vector<RectangleSumPoint> points(n);
    for (auto &[x, y, weight] : points)
    {
        verify_in >> x >> y >> weight;
    }
    std::vector<RectangleSumQuery> queries(q);
    for (auto &[left, bottom, right, top] : queries)
    {
        verify_in >> left >> bottom >> right >> top;
    }
    for (std::int64_t value : rectangle_sum(points, queries))
    {
        verify_out << value << '\n';
    }
}
