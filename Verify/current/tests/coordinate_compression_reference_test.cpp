// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Coordinate_Compression.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <random>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
struct ConstOrder
{
    bool operator()(int a, int b) const
    {
        return a < b;
    }

    bool operator()(int, int)
    {
        return false;
    }
};
}

int main()
{
    const sirkit::CoordinateCompression<int> empty;
    assert(empty.empty() && empty.size() == 0 && empty.values().empty());
    assert(empty.lower_bound(42) == 0 && empty.upper_bound(42) == 0 && !empty.find(42));
    const sirkit::CoordinateCompression initialized_empty(std::vector<int>{});
    assert(initialized_empty.lower_bound(9) == 0 && initialized_empty.upper_bound(0) == 0 && !initialized_empty.find(7));

    const std::array recorded{std::numeric_limits<std::int64_t>::min(), std::int64_t{0},
                              std::numeric_limits<std::int64_t>::max(), std::int64_t{0}};
    const sirkit::CoordinateCompression descending(std::span{recorded}, std::greater<>{});
    assert(descending.size() == 3 && descending.value(0) == recorded[2]);
    assert(descending.find(0) == 1 && descending.lower_bound(0) == 1 && descending.upper_bound(0) == 2);
    assert(descending.lower_bound(1) == 1 && descending.upper_bound(1) == 1 && !descending.find(1));

    const sirkit::CoordinateCompression const_order(std::vector{3, 1, 2, 2}, ConstOrder{});
    assert(const_order.size() == 3 && const_order.find(2) == 1 && const_order.value(2) == 3);

    sirkit::CoordinateCompression words(std::vector<std::string>{"hi", "aa", "ant", "hi"});
    auto copy = words;
    assert(copy.values().data() == words.values().data());
    auto moved = std::move(copy);
    assert(copy.empty());
    words = {};
    assert(moved.find(std::string_view("ant")) == 1 && moved.value(2) == "hi");
    assert(!moved.find(std::string_view("as")) && moved.upper_bound(std::string_view("xy")) == 3);

#ifndef SKIP_RUNTIME_CHECKS
    bool caught = false;
    try
    {
        static_cast<void>(descending.value(3));
    }
    catch (const std::out_of_range &)
    {
        caught = true;
    }
    assert(caught);
#endif

    std::mt19937 random(41841);
    for (int trial = 0; trial < 400; ++trial)
    {
        std::vector<int> input(random() % 61);
        std::set<int> expected;
        for (int &x : input)
        {
            x = static_cast<int>(random() % 41) - 20;
            expected.insert(x);
        }
        const sirkit::CoordinateCompression coordinates(std::move(input));
        assert(coordinates.size() == expected.size());
        std::size_t i = 0;
        for (const auto x : expected)
        {
            assert(coordinates.value(i++) == x);
        }
        for (int x = -22; x <= 22; ++x)
        {
            std::size_t before = 0;
            for (const auto y : expected)
            {
                before += y < x;
            }
            assert(coordinates.lower_bound(x) == before);
            assert(coordinates.upper_bound(x) == before + expected.contains(x));
            const auto exact = coordinates.find(x);
            assert(exact.has_value() == expected.contains(x));
            if (exact)
            {
                assert(*exact == before);
            }
        }
    }
}
