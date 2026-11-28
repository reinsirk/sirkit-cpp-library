// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n;
    std::int32_t height;
    std::uint32_t radius;
    verify_in >> n >> height >> radius;
    using Extractor = sirkit::KdTreePointExtractor;
    std::vector<Extractor::Point> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    const auto close_to_destination = [&](Extractor::Point point)
    {
        const std::int64_t x = point[0];
        const auto dy = static_cast<std::int64_t>(height) - point[1];
        const std::int64_t radius_wide = radius;
        return x * x + dy * dy <= radius_wide * radius_wide;
    };
    Extractor remaining(points);
    std::vector<std::size_t> queue;
    queue.reserve(n);
    remaining.take_in_circle({0, 0}, radius, queue);
    int jumps = 1;
    for (std::size_t begin = 0; begin < queue.size(); ++jumps)
    {
        const auto end = queue.size();
        for (; begin < end; ++begin)
        {
            const auto point = points[queue[begin]];
            if (close_to_destination(point))
            {
                verify_out << jumps + 1 << '\n';
                return 0;
            }
            remaining.take_in_circle(point, radius, queue);
        }
    }
    verify_out << "-1\n";
}
