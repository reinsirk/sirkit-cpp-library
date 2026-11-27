// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Static_Rectangle_Add_Rectangle_Sum.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

int main()
{
    using sirkit::static_rectangle_add_rectangle_sum;
    using sirkit::StaticRectangleAdd;
    using sirkit::StaticRectangleSumQuery;
    constexpr std::uint64_t kMod = 998244353;

    assert(static_rectangle_add_rectangle_sum({}, {}).empty());
    const std::vector<StaticRectangleSumQuery> probe{{0, 0, 1, 1}};
    assert(static_rectangle_add_rectangle_sum({}, probe) == std::vector<std::uint32_t>{0});
    constexpr auto kLow = std::numeric_limits<std::int64_t>::min();
    constexpr auto kHigh = std::numeric_limits<std::int64_t>::max();
    const std::vector<StaticRectangleAdd> extreme{{kLow, kLow, kHigh, kHigh, -3}};
    const std::vector<StaticRectangleSumQuery> extreme_queries{
        {kLow, kLow, kHigh, kHigh},
        {kLow, kLow, kLow + 1, kLow + 1},
        {kHigh, kHigh, kHigh, kHigh},
    };
    constexpr auto kWidthMod = std::numeric_limits<std::uint64_t>::max() % kMod;
    constexpr auto kAreaMod = (kWidthMod * kWidthMod) % kMod;
    constexpr auto kExpected = ((kMod - 3) * kAreaMod) % kMod;
    assert((static_rectangle_add_rectangle_sum(extreme, extreme_queries) ==
            std::vector<std::uint32_t>{kExpected, kMod - 3, 0}));
    assert((static_rectangle_add_rectangle_sum<12>(extreme, extreme_queries) ==
            std::vector<std::uint32_t>{9, 9, 0}));
    assert((static_rectangle_add_rectangle_sum<1>(extreme, extreme_queries) ==
            std::vector<std::uint32_t>{0, 0, 0}));
    assert((static_rectangle_add_rectangle_sum<std::numeric_limits<std::uint32_t>::max()>(extreme, extreme_queries) ==
            std::vector<std::uint32_t>{0, std::numeric_limits<std::uint32_t>::max() - 3, 0}));

    std::mt19937_64 rng(574226);
    const auto coord = [&]
    {
        return static_cast<std::int64_t>(rng() % 41) - 20;
    };
    for (int trial = 0; trial < 4000; ++trial)
    {
        std::vector<StaticRectangleAdd> additions;
        std::vector<StaticRectangleSumQuery> queries;
        const auto n = rng() % 21;
        const auto q = rng() % 19;
        for (std::size_t i = 0; i < n; ++i)
        {
            const auto [left, right] = std::minmax({coord(), coord()});
            const auto [bottom, top] = std::minmax({coord(), coord()});
            const auto weight = static_cast<std::int64_t>(rng() % 7) - 3;
            additions.push_back({left, bottom, right, top, weight});
        }
        std::vector<std::uint32_t> expected;
        for (std::size_t i = 0; i < q; ++i)
        {
            const auto [left, right] = std::minmax({coord(), coord()});
            const auto [bottom, top] = std::minmax({coord(), coord()});
            queries.push_back({left, bottom, right, top});
            std::int64_t sum = 0;
            for (const auto &rectangle : additions)
            {
                const auto width = std::min(right, rectangle.right) - std::max(left, rectangle.left);
                const auto height = std::min(top, rectangle.top) - std::max(bottom, rectangle.bottom);
                sum += std::max<std::int64_t>(0, width) * std::max<std::int64_t>(0, height) * rectangle.weight;
            }
            const auto residue = (sum % static_cast<std::int64_t>(kMod) + kMod) % kMod;
            expected.push_back(static_cast<std::uint32_t>(residue));
        }
        assert(static_rectangle_add_rectangle_sum(additions, queries) == expected);
    }

    assert(sirkit::detail::CheckedStaticRectangleAreaDoubleCount(5, 10) == 10);
    try
    {
        (void)sirkit::detail::CheckedStaticRectangleAreaDoubleCount(6, 10);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }

#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &rectangle : {StaticRectangleAdd{2, 0, 1, 2, 0}, StaticRectangleAdd{0, 2, 2, 1, 0}})
    {
        try
        {
            (void)static_rectangle_add_rectangle_sum(std::vector{rectangle}, {});
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
    for (const auto &rectangle : {StaticRectangleSumQuery{2, 0, 1, 2}, StaticRectangleSumQuery{0, 2, 2, 1}})
    {
        try
        {
            (void)static_rectangle_add_rectangle_sum({}, std::vector{rectangle});
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
#endif
}
