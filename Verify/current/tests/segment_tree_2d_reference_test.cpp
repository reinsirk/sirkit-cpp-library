// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Segment_Tree_2D.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>

int main()
{
    auto sum = [](int left, int right) noexcept
    {
        return left + right;
    };
    using Table = sirkit::SegmentTree2D<int, decltype(sum)>;
    const Table empty_rows(0, 7, 0, sum);
    const Table empty_columns(8, 0, std::span<const int>{}, 0, sum);
    assert(empty_rows.empty() && empty_rows.all_prod() == 0 && empty_rows.prod(0, 1, 0, 4) == 0);
    assert(empty_columns.empty() && empty_columns.width() == 0 && empty_columns.height() == 8);
    assert(empty_columns.prod(2, 0, 7, 0) == 0);
    const Table identity(4, 5, 0, sum);
    assert(identity.all_prod() == 0 && identity.get(1, 1) == 0);

    std::mt19937 rng(514812);
    for (std::size_t height = 1; height <= 9; ++height)
    {
        for (std::size_t width = 1; width <= 9; ++width)
        {
            std::vector<int> grid(height * width);
            for (auto &value : grid)
            {
                value = static_cast<int>(rng() % 21) - 10;
            }
            Table tree(height, width, grid, 0, sum);
            const auto view = [&](std::size_t row, std::size_t col) -> int &
            {
                return grid[row * width + col];
            };
            for (int step = 0; step < 200; ++step)
            {
                const auto row = rng() % height, col = rng() % width;
                assert(tree.get(row, col) == view(row, col));
                if (step % 3 == 0)
                {
                    const int replacement = static_cast<int>(rng() % 101) - 50;
                    tree.set(row, col, replacement);
                    view(row, col) = replacement;
                }
                auto bottom = rng() % height + 1, right = rng() % width + 1;
                const auto top = std::min(row, bottom);
                const auto left = std::min(col, right);
                int expected = 0;
                for (std::size_t i = top; i < bottom; ++i)
                {
                    for (std::size_t j = left; j < right; ++j)
                    {
                        expected += view(i, j);
                    }
                }
                assert(tree.prod(top, left, bottom, right) == expected);
                assert(tree.all_prod() == std::accumulate(grid.begin(), grid.end(), 0));
            }
            auto copied = tree;
            auto moved = std::move(copied);
            tree = std::move(moved);
            assert(copied.empty() && tree.all_prod() == std::accumulate(grid.begin(), grid.end(), 0));
        }
    }

    const auto bound = std::vector<int>().max_size();
    const std::array<std::array<std::size_t, 2>, 3> oversized_dimensions{{{bound, 1}, {1, bound}, {bound / 2, 2}}};
    for (const auto &[height, width] : oversized_dimensions)
    {
        bool oversized = false;
        try
        {
            const Table bad(height, width, 0, sum);
        }
        catch (const std::length_error &)
        {
            oversized = true;
        }
        assert(oversized);
    }

#ifndef SKIP_RUNTIME_CHECKS
    bool shape_rejected = false;
    try
    {
        const Table bad(1, 1, std::span<const int>{}, 0, sum);
    }
    catch (const std::invalid_argument &)
    {
        shape_rejected = true;
    }
    assert(shape_rejected);
    Table checked(2, 3, 0, sum);
    constexpr std::array<std::array<std::size_t, 4>, 4> kBadRects{{{2, 0, 1, 1}, {0, 2, 1, 1}, {0, 0, 3, 1}, {0, 0, 1, 4}}};
    for (const auto &[top, left, bottom, right] : kBadRects)
    {
        bool rejected = false;
        try
        {
            (void)checked.prod(top, left, bottom, right);
        }
        catch (const std::out_of_range &)
        {
            rejected = true;
        }
        assert(rejected);
    }
    constexpr std::array<std::array<std::size_t, 2>, 2> kBadPoints{{{2, 0}, {0, 3}}};
    for (const auto &[row, col] : kBadPoints)
    {
        bool rejected = false;
        try
        {
            checked.set(row, col, 5);
        }
        catch (const std::out_of_range &)
        {
            rejected = true;
        }
        assert(rejected && checked.all_prod() == 0);
    }
#endif
}
