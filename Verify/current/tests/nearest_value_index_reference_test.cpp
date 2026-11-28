// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Nearest_Value_Index.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <utility>
#include <vector>

namespace
{
using Signed = std::int64_t;
using Unsigned = std::uint64_t;

Unsigned distance(Signed left, Signed right)
{
    if (left > right)
    {
        std::swap(left, right);
    }
    return static_cast<Unsigned>(right) - static_cast<Unsigned>(left);
}

void check(const sirkit::NearestValueIndex &index, std::span<const Signed> points, Signed center)
{
    std::vector<std::pair<Unsigned, Signed>> expected;
    for (const auto point : points)
    {
        expected.emplace_back(distance(center, point), point);
    }
    std::sort(expected.begin(), expected.end());
    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        const auto found = index.kth_nearest(center, i + 1);
        assert(found && found->distance == expected[i].first && found->value == expected[i].second);
        const auto count = static_cast<std::size_t>(std::ranges::count_if(expected, [&](const auto &point)
        {
            return point.first <= expected[i].first;
        }));
        assert(index.count_within(center, expected[i].first) == count);
    }
    assert(!index.kth_nearest(center, 0) && !index.kth_nearest(center, expected.size() + 1));
}
}

int main()
{
    const sirkit::NearestValueIndex empty;
    assert(empty.empty() && empty.size() == 0);
    assert(empty.count_within(0, 0) == 0 && !empty.kth_nearest(0, 1));

    constexpr auto low = std::numeric_limits<Signed>::min();
    constexpr auto high = std::numeric_limits<Signed>::max();
    constexpr auto far = std::numeric_limits<Unsigned>::max();
    const std::array<Signed, 7> edges{0, high, 1, -1, low, low + 1, high};
    const sirkit::NearestValueIndex field(edges);
    assert(!field.empty() && field.size() == edges.size());
    assert(field.kth_nearest(low, edges.size())->distance == far);
    assert(field.count_within(low, far - 1) == 5);
    assert(field.count_within(high, far) == edges.size());
    for (const auto center : {low, low + 1, Signed{-1}, Signed{0}, Signed{1}, high - 1, high})
    {
        check(field, edges, center);
    }

    std::array<Signed, 2> mutable_points{-1, 1};
    const sirkit::NearestValueIndex copied(mutable_points);
    mutable_points[0] = 9;
    assert(copied.kth_nearest(0, 1)->value == -1 && copied.kth_nearest(0, 2)->value == 1);
    assert(copied.count_within(0, 1) == 2 && copied.count_within(0, 0) == 0);
    assert(copied.kth_nearest(-5, 2)->value == 1 && copied.kth_nearest(5, 2)->value == -1);

    std::mt19937_64 gen(691817);
    for (int round = 0; round < 900; ++round)
    {
        std::vector<Signed> points(gen() % 37);
        for (auto &point : points)
        {
            if (round % 2 == 0)
            {
                point = static_cast<Signed>(gen() % 41) - 20;
            }
            else
            {
                point = static_cast<Signed>(gen());
            }
        }
        const sirkit::NearestValueIndex index(points);
        check(index, points, static_cast<Signed>(gen() % 59) - 29);
        check(index, points, static_cast<Signed>(gen()));
    }
}
