// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

using Fenwick = sirkit::CompressedFenwickTree2D<>;

struct Operation
{
    int type = 0;
    std::int64_t x = 0, y = 0, upper_x_or_weight = 0, upper_y = 0;
};

int main()
{
    int n, q;
    verify_in >> n >> q;
    std::vector<Operation> events(static_cast<std::size_t>(n) + q);
    std::vector<Fenwick::Point> points;
    points.reserve(events.size());
    for (int i = 0; i < n; ++i)
    {
        auto &event = events[i];
        verify_in >> event.x >> event.y >> event.upper_x_or_weight;
        points.emplace_back(event.x, event.y);
    }
    for (int i = 0; i < q; ++i)
    {
        auto &event = events[n + i];
        verify_in >> event.type >> event.x >> event.y >> event.upper_x_or_weight;
        if (event.type == 0)
        {
            points.emplace_back(event.x, event.y);
        }
        else
        {
            verify_in >> event.upper_y;
        }
    }

    Fenwick tree(points);
    for (const auto &event : events)
    {
        if (event.type == 0)
        {
            tree.add({event.x, event.y}, event.upper_x_or_weight);
        }
        else
        {
            verify_out << tree.sum(event.x, event.y, event.upper_x_or_weight, event.upper_y) << '\n';
        }
    }
}
