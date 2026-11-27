// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/Rectangle_Union_Area.cpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

int main()
{
    try
    {
        sirkit::detail::rectangle_area_workspace(4, 4, 15);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    assert(sirkit::rectangle_union_area({}) == 0);
    std::mt19937 random(611);
    const auto coordinate = [&]()
    {
        return static_cast<int>(random() % 12) - 6;
    };
    for (int trial = 0; trial < 1400; ++trial)
    {
        std::vector<sirkit::AreaRectangle> rectangles;
        for (unsigned i = 0, n = random() % 40; i < n; ++i)
        {
            const auto [left, right] = std::minmax({coordinate(), coordinate()});
            const auto [bottom, top] = std::minmax({coordinate(), coordinate()});
            rectangles.push_back({left, bottom, right, top});
        }
        std::uint64_t expected = 0;
        for (int x = -6; x <= 6; ++x)
        {
            for (int y = -6; y <= 6; ++y)
            {
                expected += std::ranges::any_of(rectangles, [&](auto rect)
                                                {
                                                    return rect.left <= x && x < rect.right && rect.bottom <= y && y < rect.top;
                                                });
            }
        }
        assert(sirkit::rectangle_union_area(rectangles) == expected);
    }
    constexpr auto low = std::numeric_limits<std::int64_t>::min();
    constexpr auto high = std::numeric_limits<std::int64_t>::max();
    const sirkit::AreaRectangle strip{low, 0, high, 1}, overflow{low, 0, high, 2};
    assert(sirkit::rectangle_union_area({&strip, 1}) == std::numeric_limits<std::uint64_t>::max());
    try
    {
        sirkit::rectangle_union_area({&overflow, 1});
        assert(false);
    }
    catch (const std::overflow_error &)
    {
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto rectangle : {sirkit::AreaRectangle{1, 0, 0, 1}, {0, 1, 1, 0}})
    {
        try
        {
            sirkit::rectangle_union_area({&rectangle, 1});
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
#endif
}
