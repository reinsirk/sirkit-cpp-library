// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Sparse_Table_2D.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <random>

int main()
{
    const auto minimum = [](int lhs, int rhs)
    {
        return std::min(lhs, rhs);
    };
    using Table = sirkit::SparseTable2D<int, decltype(minimum)>;
    std::mt19937 rng(636282);
    const Table zero(std::vector<std::vector<int>>{}, minimum);
    const Table empty_columns(std::vector<std::vector<int>>(3), minimum);
    const Table empty_rows(0, 9, {}, minimum);
    assert(zero.empty() && empty_columns.empty() && empty_columns.height() == 3);
    assert(empty_rows.empty() && empty_rows.width() == 9 && empty_rows.height() == 0);
    for (std::size_t height = 1; height <= 9; ++height)
    {
        for (std::size_t width = 1; width <= 10; ++width)
        {
            std::vector<int> flat(height * width);
            std::vector<std::vector<int>> grid(height, std::vector<int>(width));
            for (std::size_t row = 0; row < height; ++row)
            {
                for (std::size_t col = 0; col < width; ++col)
                {
                    grid[row][col] = static_cast<int>(rng() % 100) - 50;
                    flat[row * width + col] = grid[row][col];
                }
            }
            const Table nested(grid, minimum);
            const Table contiguous(height, width, flat, minimum);
            assert(nested.height() == height && nested.width() == width && !contiguous.empty());
            for (std::size_t top = 0; top < height; ++top)
            {
                for (std::size_t left = 0; left < width; ++left)
                {
                    for (std::size_t bottom = top + 1; bottom <= height; ++bottom)
                    {
                        for (std::size_t right = left + 1; right <= width; ++right)
                        {
                            int expected = std::numeric_limits<int>::max();
                            for (std::size_t r = top; r < bottom; ++r)
                            {
                                for (std::size_t c = left; c < right; ++c)
                                {
                                    expected = std::min(expected, grid[r][c]);
                                }
                            }
                            assert(nested.prod(top, left, bottom, right) == expected);
                            assert(contiguous.prod(top, left, bottom, right) == expected);
                        }
                    }
                }
            }
        }
    }

    bool area_rejected = false;
    try
    {
        const Table oversized(std::numeric_limits<std::size_t>::max(), 2, {}, minimum);
    }
    catch (const std::length_error &)
    {
        area_rejected = true;
    }
    assert(area_rejected);
#ifndef SKIP_RUNTIME_CHECKS
    bool flat_rejected = false;
    try
    {
        const Table incomplete(2, 2, {}, minimum);
    }
    catch (const std::invalid_argument &)
    {
        flat_rejected = true;
    }
    assert(flat_rejected);
    bool ragged_rejected = false;
    try
    {
        const Table ragged(std::vector<std::vector<int>>{{3}, {8, 9}}, minimum);
    }
    catch (const std::invalid_argument &)
    {
        ragged_rejected = true;
    }
    assert(ragged_rejected);
    Table grid(std::vector<std::vector<int>>{{5, 3}, {8, 7}}, minimum);
    constexpr std::array<std::array<std::size_t, 4>, 4> kInvalid{{{0, 0, 0, 1}, {0, 0, 1, 0}, {0, 0, 3, 1}, {0, 0, 1, 3}}};
    for (const auto &[top, left, bottom, right] : kInvalid)
    {
        bool rejected = false;
        try
        {
            (void)grid.prod(top, left, bottom, right);
        }
        catch (const std::out_of_range &)
        {
            rejected = true;
        }
        assert(rejected);
    }
    const Table moved = std::move(grid);
    assert(moved.prod(0, 0, 2, 2) == 3);
    bool moved_rejected = false;
    try
    {
        (void)grid.prod(0, 0, 1, 1);
    }
    catch (const std::out_of_range &)
    {
        moved_rejected = true;
    }
    assert(moved_rejected);
#endif
}
