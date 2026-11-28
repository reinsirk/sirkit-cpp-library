// SPDX-License-Identifier: CC0-1.0
#include "Math/Histogram_Rook_Cover.cpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace
{
std::uint32_t brute(const std::vector<std::uint64_t> &heights)
{
    std::vector<std::pair<std::size_t, std::uint64_t>> cells;
    for (std::size_t column = 0; column < heights.size(); ++column)
    {
        for (std::uint64_t row = 0; row < heights[column]; ++row)
        {
            cells.emplace_back(column, row);
        }
    }
    const auto count = cells.size();
    std::vector<std::uint32_t> cover(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        for (std::size_t j = 0; j < count; ++j)
        {
            const auto [from_column, from_row] = cells[i];
            const auto [to_column, to_row] = cells[j];
            const auto [left, right] = std::minmax(from_column, to_column);
            bool visible = from_row == to_row;
            for (std::size_t column = left; visible && column <= right; ++column)
            {
                visible = heights[column] > from_row;
            }
            if (from_column == to_column || visible)
            {
                cover[i] |= std::uint32_t{1} << j;
            }
        }
    }
    const auto all = (std::uint32_t{1} << count) - 1;
    std::vector<std::uint32_t> chosen(all + 1);
    auto result = static_cast<std::uint32_t>(all == 0);
    for (std::uint32_t mask = 1; mask <= all; ++mask)
    {
        chosen[mask] = chosen[mask & (mask - 1)] | cover[static_cast<std::size_t>(std::countr_zero(mask))];
        result += chosen[mask] == all;
    }
    return result;
}

bool all_small(std::size_t at, std::vector<std::uint64_t> &heights)
{
    if (at == heights.size())
    {
        return sirkit::count_histogram_rook_covers(heights) == brute(heights);
    }
    for (std::uint64_t height = 0; height <= 4; ++height)
    {
        heights[at] = height;
        if (!all_small(at + 1, heights))
        {
            return false;
        }
    }
    return true;
}

std::uint64_t single_column(std::uint64_t height)
{
    constexpr std::uint64_t modulus = 998244353;
    // Fermat's theorem allows a different exponent domain from the library's full-width exponent.
    auto reduced = height % (modulus - 1);
    std::uint64_t current = 2, value = 1;
    while (reduced != 0)
    {
        value = value * (reduced % 2 == 0 ? 1 : current) % modulus;
        current = current * current % modulus;
        reduced /= 2;
    }
    return (value + modulus - 1) % modulus;
}
}

int main()
{
    for (std::size_t width = 0; width <= 4; ++width)
    {
        std::vector<std::uint64_t> heights(width);
        if (!all_small(0, heights))
        {
            return 1;
        }
    }
    constexpr auto largest = std::numeric_limits<std::uint64_t>::max();
    constexpr std::uint64_t modulus = 998244353;
    const auto one = single_column(largest);
    const std::vector<std::uint64_t> single{largest}, two{largest, largest}, split{largest, 0, largest};
    if (sirkit::count_histogram_rook_covers(single) != one ||
        sirkit::count_histogram_rook_covers(two) != (one * one + 2) % modulus ||
        sirkit::count_histogram_rook_covers(split) != one * one % modulus)
    {
        return 2;
    }
}
