// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Alien_Trick.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

using Cost = std::int64_t;
using Optimum = sirkit::CardinalityOptimum<Cost>;

static void check_costs(std::span<const Cost> values, std::size_t first)
{
    const auto evaluate = [&](Cost penalty)
    {
        auto best = values.front() + penalty * static_cast<Cost>(first);
        std::size_t least = first, most = first;
        for (std::size_t i = 1; i < values.size(); ++i)
        {
            const auto cost = values[i] + penalty * static_cast<Cost>(first + i);
            if (cost < best)
            {
                best = cost;
                least = first + i;
                most = least;
            }
            else if (cost == best)
            {
                most = first + i;
            }
        }
        return Optimum{best, least, most};
    };
    for (std::size_t count = 0; count < first + values.size() + 1; ++count)
    {
        const auto result = sirkit::alien_minimum_cost(count, -80, 80, evaluate);
        if (count < first || count >= first + values.size())
        {
            assert(!result);
        }
        else
        {
            assert(result == values[count - first]);
        }
    }
}

int main()
{
    const Optimum negative{-3, 1, 1}, positive{12, 0, 2};
    const Optimum tie_left{7, 2, 5}, tie_right{7, 0, 9};
    assert(sirkit::choose_cardinality_optimum(negative, positive).cost == -3);
    assert(sirkit::choose_cardinality_optimum(positive, negative).cost == -3);
    const auto tie = sirkit::choose_cardinality_optimum(tie_left, tie_right);
    assert(tie.cost == 7 && tie.least == 0 && tie.most == 9);

    check_costs(std::vector<Cost>{5}, 0);
    check_costs(std::vector<Cost>{0, 2, 4, 6}, 0);
    check_costs(std::vector<Cost>{3, 1, -1, -3}, 2);
    std::mt19937 rng(321837);
    for (int trial = 0; trial < 700; ++trial)
    {
        std::vector<Cost> deltas(rng() % 20);
        for (auto &delta : deltas)
        {
            delta = static_cast<int>(rng() % 141) - 70;
        }
        std::ranges::sort(deltas);
        std::vector<Cost> costs{static_cast<int>(rng() % 101) - 50};
        for (auto delta : deltas)
        {
            costs.push_back(costs.back() + delta);
        }
        check_costs(costs, rng() % 6);
    }

    const auto evaluate = [](Cost penalty) -> Optimum
    {
        if (penalty < 0)
        {
            return {penalty, 1, 1};
        }
        return {0, 0, static_cast<std::size_t>(penalty == 0)};
    };
    constexpr auto kLow = std::numeric_limits<Cost>::min(), kHigh = std::numeric_limits<Cost>::max();
    assert(sirkit::alien_minimum_cost(0, kLow, kHigh, evaluate) == 0);
    assert(sirkit::alien_minimum_cost(1, kLow, kHigh, evaluate) == 0);
    assert(!sirkit::alien_minimum_cost(1, 2, 8, evaluate));
    assert(!sirkit::alien_minimum_cost(0, -10, -6, evaluate));

    bool caught = false;
    try
    {
        (void)sirkit::alien_minimum_cost(1, 5, 3, evaluate);
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught);
    caught = false;
    try
    {
        (void)sirkit::alien_minimum_cost(0, 0, 1, [](Cost)
                                         {
                                             return Optimum{0, 7, 3};
                                         });
    }
    catch (const std::invalid_argument &)
    {
        caught = true;
    }
    assert(caught);
}
