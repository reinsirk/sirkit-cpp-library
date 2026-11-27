// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Assignment.cpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

int main()
{
    assert(sirkit::minimum_assignment({}).cost == 0);
    std::mt19937 random(753);
    for (std::size_t rows = 1; rows <= 5; ++rows)
    {
        for (std::size_t cols = rows; cols <= 6; ++cols)
        {
            for (int trial = 0; trial < 80; ++trial)
            {
                std::vector<std::vector<std::int64_t>> cost(rows, std::vector<std::int64_t>(cols));
                for (auto &row : cost)
                {
                    for (auto &value : row)
                    {
                        value = static_cast<std::int64_t>(random() % 15) - 7;
                    }
                }
                const auto result = sirkit::minimum_assignment(cost);
                auto expected = std::numeric_limits<std::int64_t>::max();
                std::vector<std::size_t> permutation(cols);
                std::iota(permutation.begin(), permutation.end(), std::size_t{0});
                do
                {
                    std::int64_t sum = 0;
                    for (std::size_t i = 0; i < rows; ++i)
                    {
                        sum += cost[i][permutation[i]];
                    }
                    expected = std::min(expected, sum);
                } while (std::ranges::next_permutation(permutation).found);
                auto columns = result.column;
                std::ranges::sort(columns);
                assert(columns.size() == rows && columns.back() < cols);
                assert(std::ranges::adjacent_find(columns) == columns.end());
                std::int64_t sum = 0;
                for (std::size_t i = 0; i < rows; ++i)
                {
                    sum += cost[i][result.column[i]];
                }
                assert(sum == expected && result.cost == expected);
            }
        }
    }
    constexpr auto kLimit = std::numeric_limits<std::int64_t>::max() / 7;
    const std::vector<std::vector<std::int64_t>> wide{{kLimit, -kLimit}, {-kLimit, kLimit}};
    assert(sirkit::minimum_assignment(wide).cost == -2 * kLimit);
    try
    {
        const std::vector<std::vector<std::int64_t>> narrow{{1}, {2}};
        sirkit::minimum_assignment(narrow);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    for (auto value : {std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max()})
    {
        try
        {
            const std::vector<std::vector<std::int64_t>> costs{{value}};
            sirkit::minimum_assignment(costs);
            assert(false);
        }
        catch (const std::overflow_error &)
        {
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    try
    {
        const std::vector<std::vector<std::int64_t>> costs{{1, 2}, {3}};
        sirkit::minimum_assignment(costs);
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
