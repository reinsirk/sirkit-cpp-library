// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Alien_Trick.cpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>

int main()
{
    // k個選んだときの最小費用は 0,2,5,8,12。差分 2,3,3,4 が単調。
    constexpr std::array costs{0, 2, 5, 8, 12};
    const auto evaluator = [&](std::int64_t penalty)
    {
        sirkit::CardinalityOptimum<std::int64_t> best{};
        for (std::size_t count = 1; count < costs.size(); ++count)
        {
            const auto cost = costs[count] + penalty * static_cast<std::int64_t>(count);
            best = sirkit::choose_cardinality_optimum(best, {cost, count, count});
        }
        return best;
    };
    const auto result = sirkit::alien_minimum_cost(2, -4, 0, evaluator);
    std::cout << *result << '\n'; // 5
}
