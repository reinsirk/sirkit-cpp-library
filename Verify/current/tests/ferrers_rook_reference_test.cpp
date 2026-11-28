// SPDX-License-Identifier: CC0-1.0
#include "Math/Ferrers_Rook.cpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
std::vector<std::uint32_t> brute_rook(const std::vector<std::uint32_t> &heights)
{
    const auto rows = heights.empty() ? 0 : *std::ranges::max_element(heights);
    std::vector<std::uint32_t> masks(std::size_t{1} << rows);
    masks[0] = 1;
    for (const auto height : heights)
    {
        auto next = masks;
        for (std::size_t mask = 0; mask < masks.size(); ++mask)
        {
            for (std::uint32_t row = 0; row < height; ++row)
            {
                const auto bit = std::size_t{1} << row;
                if ((mask & bit) == 0)
                {
                    next[mask | bit] += masks[mask];
                }
            }
        }
        masks = std::move(next);
    }
    std::vector<std::uint32_t> result(heights.size() + 1);
    for (std::size_t mask = 0; mask < masks.size(); ++mask)
    {
        const auto occupied = static_cast<std::size_t>(std::popcount(mask));
        if (occupied < result.size())
        {
            result[occupied] += masks[mask];
        }
    }
    return result;
}

std::vector<std::uint32_t> brute_hits(const std::vector<std::uint32_t> &heights)
{
    std::vector<std::uint32_t> order(heights.size()), result(heights.size() + 1);
    std::iota(order.begin(), order.end(), 0);
    do
    {
        std::size_t hits = 0;
        for (std::size_t i = 0; i < heights.size(); ++i)
        {
            hits += order[i] < heights[i];
        }
        ++result[hits];
    } while (std::next_permutation(order.begin(), order.end()));
    return result;
}
}

int main()
{
    const std::vector<std::uint32_t> none;
    if (sirkit::ferrers_rook_counts(none) != std::vector<std::uint32_t>{1} ||
        sirkit::ferrers_hit_counts(none) != std::vector<std::uint32_t>{1})
    {
        return 1;
    }
    std::mt19937 random(50739);
    for (std::size_t n = 1; n <= 7; ++n)
    {
        for (int repetition = 0; repetition < 90; ++repetition)
        {
            std::vector<std::uint32_t> board(n), rectangle(n);
            for (std::size_t i = 0; i < n; ++i)
            {
                board[i] = static_cast<std::uint32_t>(random() % (n + 1));
                rectangle[i] = static_cast<std::uint32_t>(random() % (9 - n / 4));
            }
            if (sirkit::ferrers_rook_counts(rectangle) != brute_rook(rectangle) ||
                sirkit::ferrers_rook_counts(board) != brute_rook(board) ||
                sirkit::ferrers_hit_counts(board) != brute_hits(board))
            {
                return 2;
            }
        }
    }
    constexpr auto upper = std::numeric_limits<std::uint32_t>::max();
    constexpr std::uint64_t modulus = 998244353;
    const std::vector<std::uint32_t> huge{upper, upper - 11};
    const std::vector<std::uint32_t> expected{1, static_cast<std::uint32_t>((2 * std::uint64_t{upper} - 11) % modulus),
                                              static_cast<std::uint32_t>((std::uint64_t{upper - 1} * (upper - 11)) % modulus)};
    if (sirkit::ferrers_rook_counts(huge) != expected)
    {
        return 3;
    }
    try
    {
        static_cast<void>(sirkit::ferrers_hit_counts(huge));
        return 4;
    }
    catch (const std::invalid_argument &)
    {
    }
    try
    {
        const std::vector<std::uint32_t> oversized(std::size_t{1} << 20);
        static_cast<void>(sirkit::ferrers_rook_counts(oversized));
        return 5;
    }
    catch (const std::length_error &)
    {
    }
}
