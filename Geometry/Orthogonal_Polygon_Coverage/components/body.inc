#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_GEO_ORTHOGONAL_POLYGON_COVERAGE_DEFINED
#define SIRKIT_GEO_ORTHOGONAL_POLYGON_COVERAGE_DEFINED

namespace geo
{

using OrthogonalPoint = std::pair<std::int64_t, std::int64_t>;
using OrthogonalPolygon = std::vector<OrthogonalPoint>;

inline std::vector<std::int64_t> orthogonal_polygon_coverage(
    std::span<const OrthogonalPolygon> polygons, std::span<const OrthogonalPoint> cells)
{
    struct Crossing
    {
        std::int64_t x = 0;
        std::int64_t low = 0;
        std::int64_t high = 0;
        int sign = 0;
    };
    std::vector<Crossing> events;
    for (const auto &polygon : polygons)
    {
        std::size_t count = polygon.size();
        if (count > 1 && polygon.front() == polygon.back())
        {
            --count;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (count < 4)
        {
            throw std::invalid_argument("orthogonal_polygon_coverage: fewer than four vertices");
        }
#endif
        const auto minimum = std::min_element(polygon.begin(), polygon.begin() + count);
        const auto next = (static_cast<std::size_t>(minimum - polygon.begin()) + 1) % count;
        // At the leftmost lowest corner the two incident sides lead east and north.
        const bool counterclockwise = polygon[next].first > minimum->first;
        for (std::size_t i = 0; i < count; ++i)
        {
            const auto &from = polygon[i];
            const auto &to = polygon[(i + 1) % count];
            const bool vertical = from.first == to.first;
#ifndef SKIP_RUNTIME_CHECKS
            if (vertical == (from.second == to.second))
            {
                throw std::invalid_argument("orthogonal_polygon_coverage: invalid edge");
            }
#endif
            if (vertical)
            {
                const auto [low, high] = std::minmax(from.second, to.second);
                const bool up = from.second < to.second;
                events.push_back({from.first, low, high, up == counterclockwise ? -1 : 1});
            }
        }
    }
    std::sort(events.begin(), events.end(), [](const Crossing &a, const Crossing &b)
              {
                  return a.x < b.x;
              });
    std::vector<std::size_t> order(cells.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b)
              {
                  return cells[a].first < cells[b].first;
              });

    std::vector<std::int64_t> ys;
    ys.reserve(cells.size());
    for (const auto &cell : cells)
    {
        ys.push_back(cell.second);
    }
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
    ::reinsirk::detail::CheckedFenwickStorageSize(ys.size(), std::vector<std::uint64_t>{}.max_size());
    fenwick_tree<std::int64_t> active(static_cast<int>(ys.size()));

    const auto rank = [&](std::int64_t y)
    {
        return static_cast<int>(std::lower_bound(ys.begin(), ys.end(), y) - ys.begin());
    };
    std::vector<std::int64_t> answers(cells.size());
    auto event = events.begin();
    for (const auto id : order)
    {
        const auto &[x, y] = cells[id];
        while (event != events.end() && event->x <= x)
        {
            const int low = rank(event->low), high = rank(event->high);
            if (low < high)
            {
                active.add(low, event->sign);
                if (high < active.size())
                {
                    active.add(high, -event->sign);
                }
            }
            ++event;
        }
        answers[id] = active.sum(0, rank(y) + 1);
    }
    return answers;
}

}

#endif
