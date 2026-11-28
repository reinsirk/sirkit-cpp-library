// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Exact_Cover.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

static void check_cover(std::size_t required, std::size_t extra, const std::vector<std::vector<std::size_t>> &rows)
{
    sirkit::ExactCover cover(required, extra);
    for (std::size_t id = 0; id < rows.size(); ++id)
    {
        assert(cover.add_row(rows[id]) == id);
    }

    std::vector<std::uint32_t> expected;
    const auto combinations = std::uint32_t{1} << rows.size();
    for (std::uint32_t mask = 0; mask < combinations; ++mask)
    {
        std::vector<unsigned> occurrences(required + extra);
        for (std::size_t id = 0; id < rows.size(); ++id)
        {
            if ((mask >> id & 1u) != 0)
            {
                for (auto column : rows[id])
                {
                    ++occurrences[column];
                }
            }
        }
        bool valid = true;
        for (std::size_t column = 0; column < occurrences.size(); ++column)
        {
            valid &= column < required ? occurrences[column] == 1 : occurrences[column] <= 1;
        }
        if (valid)
        {
            expected.push_back(mask);
        }
    }

    const auto mask_of = [&](std::span<const std::size_t> chosen)
    {
        std::uint32_t mask = 0;
        for (auto id : chosen)
        {
            assert(id < rows.size() && ((mask >> id) & 1u) == 0);
            mask |= std::uint32_t{1} << id;
        }
        return mask;
    };
    std::vector<std::uint32_t> actual;
    cover.for_each_solution([&](std::span<const std::size_t> chosen)
                            {
                                actual.push_back(mask_of(chosen));
                                return true;
                            });
    std::ranges::sort(actual);
    assert(actual == expected);
    const auto found = cover.solve();
    assert(found.has_value() == !expected.empty());
    if (found)
    {
        assert(std::ranges::binary_search(expected, mask_of(*found)));
    }
}

static void check_queens()
{
    constexpr auto kSize = std::size_t{8};
    constexpr auto kDiagonals = 2 * kSize - 1;
    sirkit::ExactCover cover(2 * kSize, 2 * kDiagonals);
    for (std::size_t row = 0; row < kSize; ++row)
    {
        for (std::size_t column = 0; column < kSize; ++column)
        {
            cover.add_row(std::array{row, kSize + column, 2 * kSize + row + column,
                                     2 * kSize + kDiagonals + kSize - 1 + row - column});
        }
    }
    std::size_t count = 0;
    cover.for_each_solution([&](std::span<const std::size_t> rows)
                            {
                                assert(rows.size() == kSize);
                                ++count;
                                return true;
                            });
    assert(count == 92);

    count = 0;
    cover.for_each_solution([&](std::span<const std::size_t>)
                            {
                                return ++count != 2;
                            });
    assert(count == 2);
    bool caught = false;
    try
    {
        cover.for_each_solution([](std::span<const std::size_t>) -> bool
                                {
                                    throw std::runtime_error("stop");
                                });
    }
    catch (const std::runtime_error &)
    {
        caught = true;
    }
    assert(caught);
    assert(cover.solve()->size() == kSize);
}

int main()
{
    check_cover(0, 0, {});
    check_cover(0, 5, {});
    check_cover(4, 0, {});
    check_cover(2, 1, {{2, 0}, {1, 2}, {1}, {0, 1}, {1}});
    check_cover(2, 1, {{2, 0}, {1, 2}});
    check_cover(3, 0, {{0}, {0}, {1}, {1}, {2}, {2}, {0, 1, 2}});
    std::mt19937 rng(3968551);
    for (int trial = 0; trial < 430; ++trial)
    {
        const std::size_t required = 1 + rng() % 6, extra = rng() % 4, count = rng() % 13;
        std::vector<std::vector<std::size_t>> rows(count);
        for (auto &columns : rows)
        {
            const auto force = rng() % required;
            for (std::size_t column = 0; column < required + extra; ++column)
            {
                if (column == force || rng() % 4 == 0)
                {
                    columns.push_back(column);
                }
            }
            std::ranges::reverse(columns);
        }
        check_cover(required, extra, rows);
    }
    check_queens();

    sirkit::ExactCover validated(3, 2);
    for (const auto &bad : std::vector<std::vector<std::size_t>>{{}, {3}, {0, 5}, {0, 0, 1}})
    {
        bool caught = false;
        try
        {
            (void)validated.add_row(bad);
        }
        catch (const std::invalid_argument &)
        {
            caught = true;
        }
        assert(caught);
    }
    assert(validated.add_row(std::array<std::size_t, 3>{0, 1, 2}) == 0);
    assert(validated.solve()->size() == 1);

    constexpr auto kMax = std::numeric_limits<std::size_t>::max();
    for (const auto &counts : {std::array{kMax, std::size_t{0}}, std::array{std::size_t{5}, kMax}})
    {
        bool caught = false;
        try
        {
            (void)sirkit::ExactCover(counts[0], counts[1]);
        }
        catch (const std::length_error &)
        {
            caught = true;
        }
        assert(caught);
    }
}
