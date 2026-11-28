// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Knapsack_By_Value.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

using Item = sirkit::KnapsackByValue::Item;

static void check(const std::vector<Item> &items)
{
    const sirkit::KnapsackByValue knapsack(items);
    std::size_t max_value = 0;
    for (const auto &item : items)
    {
        max_value += item.value;
    }
    std::vector<std::optional<std::uint64_t>> expected(max_value + 1);
    for (std::size_t mask = 0; mask < (std::size_t{1} << items.size()); ++mask)
    {
        std::size_t value = 0;
        std::uint64_t weight = 0;
        for (std::size_t i = 0; i < items.size(); ++i)
        {
            if ((mask & (std::size_t{1} << i)) != 0)
            {
                value += items[i].value;
                weight += items[i].weight;
            }
        }
        auto &candidate = expected[value];
        if (!candidate || weight < *candidate)
        {
            candidate = weight;
        }
    }
    for (std::size_t value = 0; value < expected.size(); ++value)
    {
        assert(knapsack.minimum_weight(value) == expected[value]);
    }
    assert(!knapsack.minimum_weight(expected.size()));
    for (std::uint64_t budget = 0; budget <= 50; ++budget)
    {
        std::size_t best = 0;
        for (std::size_t value = 0; value < expected.size(); ++value)
        {
            if (expected[value] && *expected[value] <= budget)
            {
                best = value;
            }
        }
        assert(knapsack.max_value(budget) == best);
    }
}

int main()
{
    check({});
    check({{0, 0}, {3, 0}, {0, 3}, {4, 2}});
    check({{3, 3}, {1, 3}, {4, 3}});
    check({{3, 15}, {5, 10}, {7, 20}});
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const std::array huge{Item{kMax, 3}, Item{1, 5}};
    const sirkit::KnapsackByValue knapsack(huge);
    assert(knapsack.minimum_weight(3) == kMax);
    assert(!knapsack.minimum_weight(8));
    assert(knapsack.max_value(kMax) == 5);
    assert(knapsack.max_value(0) == 0);
    const std::array free{Item{0, 6}, Item{kMax, 3}};
    assert(sirkit::KnapsackByValue(free).minimum_weight(9) == kMax);
    assert(sirkit::KnapsackByValue(free).max_value(0) == 6);
    bool caught = false;
    try
    {
        const std::array oversized{Item{1, std::numeric_limits<std::size_t>::max()}};
        (void)sirkit::KnapsackByValue(oversized);
    }
    catch (const std::length_error &)
    {
        caught = true;
    }
    assert(caught);
    std::mt19937 rng(83942);
    for (std::size_t n = 0; n <= 11; ++n)
    {
        for (int t = 0; t < 29; ++t)
        {
            std::vector<Item> items(n);
            for (auto &item : items)
            {
                item = {rng() % 13, rng() % 19};
            }
            check(items);
        }
    }
    const std::vector<Item> maximum(100, {10'000'000, 1000});
    assert(sirkit::KnapsackByValue(maximum).max_value(1'000'000'000) == 100'000);
}
