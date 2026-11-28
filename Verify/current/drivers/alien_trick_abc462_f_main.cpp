// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

int main()
{
    std::size_t cases{};
    verify_in >> cases;
    for (std::size_t i = 0; i < cases; ++i)
    {
        std::string text;
        std::size_t add{};
        verify_in >> text >> add;
        std::vector<int> costs(text.size() - 2);
        for (std::size_t position = 0; position + 2 < text.size(); ++position)
        {
            const int edits = (text[position] != 'A') + (text[position + 1] != 'B') + (text[position + 2] != 'C');
            costs[position] = edits;
            add += edits == 0;
        }
        if (add > text.size() / 3)
        {
            verify_out << -1 << '\n';
            continue;
        }
        const auto evaluate = [&](std::int64_t penalty)
        {
            std::array<sirkit::CardinalityOptimum<std::int64_t>, 3> dp{};
            for (std::size_t j = 0; j < costs.size(); ++j)
            {
                auto take = dp[j % 3];
                take.cost += penalty + costs[j];
                ++take.least;
                ++take.most;
                dp[j % 3] = sirkit::choose_cardinality_optimum(dp[(j + 2) % 3], take);
            }
            return dp[(costs.size() - 1) % 3];
        };
        const auto lower = -static_cast<std::int64_t>(text.size()) - 1;
        verify_out << sirkit::alien_minimum_cost(add, lower, 0, evaluate).value_or(-1) << '\n';
    }
}
