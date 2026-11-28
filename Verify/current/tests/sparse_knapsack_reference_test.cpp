// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Sparse_Knapsack.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

using Item = sirkit::SparseKnapsack::Item;

static void check(std::uint64_t capacity, const std::vector<Item> &items)
{
    std::vector<std::uint64_t> exact(capacity + 1);
    for (std::size_t mask = 0; mask < (std::size_t{1} << items.size()); ++mask)
    {
        std::uint64_t weight = 0, value = 0;
        for (std::size_t i = 0; i < items.size(); ++i)
        {
            if ((mask & (std::size_t{1} << i)) != 0)
            {
                weight += items[i].weight;
                value += items[i].value;
            }
        }
        if (weight <= capacity)
        {
            exact[weight] = std::max(exact[weight], value);
        }
    }
    const sirkit::SparseKnapsack obtained(capacity, items);
    const auto frontier = obtained.frontier();
    std::size_t next = 0;
    std::uint64_t best = 0;
    for (std::uint64_t weight = 0; weight <= capacity; ++weight)
    {
        if (exact[weight] > best)
        {
            best = exact[weight];
            assert(next < frontier.size());
            assert(frontier[next].weight == weight && frontier[next].value == best);
            ++next;
        }
        assert(obtained.max_value(weight) == best);
    }
    assert(next == frontier.size() && obtained.max_value() == best);
}

template <class Exception, class Test>
static void throws(const Test &test)
{
    bool caught = false;
    try
    {
        test();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

int main()
{
    check(0, {});
    check(0, {{5, 8}, {0, 1}, {0, 2}, {0, 0}});
    check(6, {{5, 100}, {2, 1}, {3, 20}});
    check(25, {{10, 7}, {6, 100}, {4, 25}, {3, 11}});
    check(9, {{5, 50}, {5, 40}, {5, 0}, {3, 25}, {9, 100}});
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const std::array extremes{Item{kMax, 2}, Item{1, 3}, Item{1, 0}};
    assert(sirkit::SparseKnapsack(kMax, extremes).max_value() == 3);
    const std::array huge_free{Item{0, kMax}};
    assert(sirkit::SparseKnapsack(kMax, huge_free).max_value() == kMax);
    throws<std::overflow_error>([&]
                                {
                                    const std::array items{Item{0, kMax}, Item{1, 1}};
                                    (void)sirkit::SparseKnapsack(1, items);
                                });
    throws<std::out_of_range>([]
                              {
                                  (void)sirkit::SparseKnapsack(0, {}).max_value(1);
                              });
    std::mt19937 rng(20261128);
    for (int i = 0; i < 650; ++i)
    {
        std::vector<Item> items(rng() % 12);
        for (auto &item : items)
        {
            item = {rng() % 20, rng() % 53};
        }
        check(rng() % 62, items);
    }
}
