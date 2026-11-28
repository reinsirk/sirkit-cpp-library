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
    assert(sirkit::rectangle_union_perimeter({}) == 0);
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
        bool occupied[14][14]{};
        for (int x = 1; x < 13; ++x)
        {
            for (int y = 1; y < 13; ++y)
            {
                occupied[x][y] = std::ranges::any_of(rectangles, [&](auto rect)
                                                     {
                                                         const int cx = x - 7, cy = y - 7;
                                                         return rect.left <= cx && cx < rect.right && rect.bottom <= cy && cy < rect.top;
                                                     });
            }
        }
        std::uint64_t area = 0, perimeter = 0;
        for (int x = 1; x < 13; ++x)
        {
            for (int y = 1; y < 13; ++y)
            {
                if (occupied[x][y])
                {
                    ++area;
                    perimeter += !occupied[x - 1][y] + !occupied[x + 1][y] + !occupied[x][y - 1] + !occupied[x][y + 1];
                }
            }
        }
        assert(sirkit::rectangle_union_area(rectangles) == area);
        assert(sirkit::rectangle_union_perimeter(rectangles) == perimeter);
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
    constexpr std::int64_t kQuarter = std::int64_t{1} << 62;
    const sirkit::AreaRectangle overflowing_area_only{0, 0, 10'000'000'000LL, 10'000'000'000LL};
    assert(sirkit::rectangle_union_perimeter({&overflowing_area_only, 1}) == 40'000'000'000ULL);
    for (const auto rect : {strip, sirkit::AreaRectangle{0, low, 1, high}, {0, 0, kQuarter, kQuarter}})
    {
        try
        {
            (void)sirkit::rectangle_union_perimeter({&rect, 1});
            assert(false);
        }
        catch (const std::overflow_error &)
        {
        }
    }
    const std::vector<sirkit::AreaRectangle> ring{{0, 0, 3, 1}, {0, 2, 3, 3}, {0, 1, 1, 2}, {2, 1, 3, 2}};
    assert(sirkit::rectangle_union_perimeter(ring) == 16);
    constexpr int kCount = 100000;
    std::vector<sirkit::AreaRectangle> large;
    large.reserve(kCount);
    for (int x = 0; x < kCount; ++x)
    {
        large.push_back({x, 0, x + 1, 2});
    }
    assert(sirkit::rectangle_union_perimeter(large) == 2 * kCount + 4);
    for (int x = 0; x < kCount; ++x)
    {
        large[x].left = 2 * x;
        large[x].right = 2 * x + 1;
    }
    assert(sirkit::rectangle_union_perimeter(large) == 6 * kCount);
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
        try
        {
            (void)sirkit::rectangle_union_perimeter({&rectangle, 1});
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
#endif
}
