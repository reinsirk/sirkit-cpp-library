// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Pair_Product_Order.cpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
void check(const sirkit::PairProductOrder &actual, std::vector<std::int64_t> expected)
{
    std::sort(expected.begin(), expected.end());
    assert(actual.size() == expected.size());
    assert(!actual.kth_smallest(0));
    assert(!actual.kth_smallest(actual.size() + 1));
    for (std::size_t rank = 0; rank < expected.size(); ++rank)
    {
        assert(actual.kth_smallest(rank + 1) == expected[rank]);
    }
    for (std::int64_t bound = -100; bound <= 100; bound += 3)
    {
        const auto it = std::upper_bound(expected.begin(), expected.end(), bound);
        assert(actual.count_at_most(bound) == static_cast<std::uint64_t>(it - expected.begin()));
    }
}

bool rank_overflows(std::int64_t first, std::int64_t second)
{
    try
    {
        const sirkit::PairProductOrder pair(std::array{first, second});
        static_cast<void>(pair.kth_smallest(1));
    }
    catch (const std::overflow_error &)
    {
        return true;
    }
    return false;
}
} // namespace

int main()
{
    std::mt19937 rng(225126);
    for (std::size_t trial = 0; trial < 360; ++trial)
    {
        std::vector<std::int64_t> left(trial % 13), right(trial % 17);
        for (auto &value : left)
        {
            value = static_cast<std::int64_t>(rng() % 21) - 10;
        }
        for (auto &value : right)
        {
            value = static_cast<std::int64_t>(rng() % 21) - 10;
        }
        std::vector<std::int64_t> expected;
        for (std::size_t i = 0; i < left.size(); ++i)
        {
            for (std::size_t j = i + 1; j < left.size(); ++j)
            {
                expected.push_back(left[i] * left[j]);
            }
        }
        check(sirkit::PairProductOrder(left), expected);
        expected.clear();
        for (const auto a : left)
        {
            for (const auto b : right)
            {
                expected.push_back(a * b);
            }
        }
        check(sirkit::PairProductOrder(left, right), expected);
    }

    constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
    const std::array<std::int64_t, 4> extremes{minimum, 1, 2, -1};
    const sirkit::PairProductOrder products(extremes);
    assert(products.count_at_most(minimum) == 2);
    assert(products.kth_smallest(2) == minimum);
    assert(products.count_at_most(maximum) == 5);
    assert(sirkit::PairProductOrder(std::array{maximum, std::int64_t{1}}).kth_smallest(1) == maximum);
    assert(rank_overflows(minimum, 2));
    assert(rank_overflows(minimum, -1));
    assert(!rank_overflows(minimum, 0));

    sirkit::PairProductOrder original(std::array<std::int64_t, 2>{2, 3});
    auto moved = std::move(original);
    assert(original.size() == 0 && !original.kth_smallest(1));
    assert(original.count_at_most(maximum) == 0);
    original = moved;
    assert(original.kth_smallest(1) == 6);
    moved = std::move(original);
    assert(moved.kth_smallest(1) == 6 && original.size() == 0);

    bool length_exception = false;
    try
    {
        sirkit::detail::check_pair_product_length(std::uint64_t{1} << 32);
    }
    catch (const std::length_error &)
    {
        length_exception = true;
    }
    assert(length_exception);
}
