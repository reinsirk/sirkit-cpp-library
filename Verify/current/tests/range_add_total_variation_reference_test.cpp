#include "../../../DataStructure/Range_Add_Total_Variation.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using sirkit::RangeAddTotalVariation;

std::uint64_t direct(const std::vector<std::int64_t> &values)
{
    std::uint64_t result = 0;
    for (std::size_t i = 1; i < values.size(); ++i)
    {
        result += static_cast<std::uint64_t>(std::abs(values[i] - values[i - 1]));
    }
    return result;
}

#ifndef SKIP_RUNTIME_CHECKS
template <typename Exception, typename Action>
void expect_exception(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}
#endif
}

int main()
{
    RangeAddTotalVariation empty;
    empty.range_add(0, 0, 1);
    assert(empty.size() == 0 && empty.total_variation() == 0);
    constexpr auto low = std::numeric_limits<std::int64_t>::min();
    constexpr auto high = std::numeric_limits<std::int64_t>::max();
    constexpr auto high_unsigned = std::numeric_limits<std::uint64_t>::max();
    RangeAddTotalVariation wide(std::array{low, high});
    assert(wide.total_variation() == high_unsigned);
    wide.range_add(0, 1, high);
    assert(wide.total_variation() == (std::uint64_t{1} << 63));
    wide.range_add(1, 2, low);
    assert(wide.total_variation() == 0);
    RangeAddTotalVariation reversed(std::array{high, low});
    assert(reversed.total_variation() == high_unsigned);
    reversed.range_add(0, 1, low);
    assert(reversed.total_variation() == static_cast<std::uint64_t>(high));
    reversed.range_add(1, 2, high);
    assert(reversed.total_variation() == 0);

    RangeAddTotalVariation original(std::array<std::int64_t, 4>{5, 2, 4, 1});
    auto copy = original;
    copy.range_add(0, 2, -2);
    assert(copy.total_variation() == 10 && original.total_variation() == 8);
    auto moved = std::move(copy);
    assert(copy.size() == 0 && copy.total_variation() == 0);
    assert(moved.size() == 4 && moved.total_variation() == 10);
    copy = original;
    moved = std::move(copy);
    assert(moved.total_variation() == 8 && copy.size() == 0);

    std::mt19937 random(64193);
    for (int test = 0; test < 650; ++test)
    {
        const auto n = static_cast<std::size_t>(random() % 43);
        std::vector<std::int64_t> values(n);
        for (auto &value : values)
        {
            value = static_cast<std::int64_t>(random() % 201) - 100;
        }
        RangeAddTotalVariation structure(values);
        assert(structure.size() == n && structure.total_variation() == direct(values));
        for (int query = 0; query < 170; ++query)
        {
            const auto a = static_cast<std::size_t>(random() % (n + 1));
            const auto b = static_cast<std::size_t>(random() % (n + 1));
            const auto left = std::min(a, b);
            const auto right = std::max(a, b);
            const auto delta = static_cast<std::int64_t>(random() % 81) - 40;
            structure.range_add(left, right, delta);
            for (std::size_t i = left; i < right; ++i)
            {
                values[i] += delta;
            }
            assert(structure.total_variation() == direct(values));
        }
    }

#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::out_of_range>([&] { original.range_add(3, 2, 1); });
    expect_exception<std::out_of_range>([&] { original.range_add(1, 5, 1); });
    assert(original.total_variation() == 8);
    expect_exception<std::overflow_error>([&] { (void)RangeAddTotalVariation(std::array{low, high, low}); });
    RangeAddTotalVariation limit(std::array{low, high});
    expect_exception<std::overflow_error>([&] { limit.range_add(1, 2, 1); });
    assert(limit.total_variation() == high_unsigned);
    limit.range_add(1, 2, low);
    assert(limit.total_variation() == static_cast<std::uint64_t>(high));
    RangeAddTotalVariation two_boundaries(std::array{std::int64_t{}, high, std::int64_t{}});
    expect_exception<std::overflow_error>([&] { two_boundaries.range_add(1, 2, 1); });
    assert(two_boundaries.total_variation() == high_unsigned - 1);
    two_boundaries.range_add(1, 2, -high);
    assert(two_boundaries.total_variation() == 0);
#endif
}
