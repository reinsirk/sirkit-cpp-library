// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Binary_Grid_Squares.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

using sirkit::BinaryGridSquare;
using sirkit::binary_grid_squares;

void expect_square(const std::optional<BinaryGridSquare> &square, int top, int left, int side)
{
    assert(square && square->top == top && square->left == left && square->side == side);
}

void boundary_cases()
{
    const auto unused = [](int, int)
    {
        assert(false);
        return false;
    };
    for (const auto &empty : {binary_grid_squares(0, 17, unused), binary_grid_squares(17, 0, unused)})
    {
        assert(empty.count == 0 && !empty.largest && empty.by_side.size() == 1 && empty.by_side[0] == 0);
    }

    const auto tied = binary_grid_squares(2, 3, [](int row, int col)
    {
        return (row == 1 && col == 0) || (row == 0 && col == 2);
    });
    expect_square(tied.largest, 0, 2, 1);
    assert(tied.count == 2 && tied.by_side[1] == 2 && tied.by_side[2] == 0);

    const auto huge = binary_grid_squares(3000, 3000, [](int, int) { return true; });
    assert(huge.count == 9004500500ULL && huge.by_side[2999] == 4 && huge.by_side[3000] == 1);
    expect_square(huge.largest, 0, 0, 3000);

    constexpr auto limit = std::numeric_limits<std::uint64_t>::max();
    assert(sirkit::detail::AddBinaryGridSquareCounts(limit - 3, 3) == limit);
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[height, width] : {std::pair{-1, 3}, std::pair{3, -1}})
    {
        bool rejected = false;
        try
        {
            static_cast<void>(binary_grid_squares(height, width, unused));
        }
        catch (const std::invalid_argument &)
        {
            rejected = true;
        }
        assert(rejected);
    }
    bool overflow = false;
    try
    {
        static_cast<void>(sirkit::detail::AddBinaryGridSquareCounts(limit, 1));
    }
    catch (const std::overflow_error &)
    {
        overflow = true;
    }
    assert(overflow);
#endif
}

void random_cases()
{
    std::mt19937 random(0xd1ca23);
    for (int test = 0; test < 1400; ++test)
    {
        const int height = static_cast<int>(random() % 10), width = static_cast<int>(random() % 10);
        const std::size_t cells = static_cast<std::size_t>(height * width);
        std::vector<unsigned char> good(cells), calls(cells);
        for (auto &value : good)
        {
            value = static_cast<unsigned char>(random() % ((test % 4) + 2) != 0);
        }
        const auto position = [width](int row, int col)
        {
            return static_cast<std::size_t>(row * width + col);
        };
        const auto actual = binary_grid_squares(height, width, [&](int row, int col)
        {
            assert(0 <= row && row < height && 0 <= col && col < width);
            const auto index = position(row, col);
            ++calls[index];
            return good[index] != 0;
        });
        assert(std::ranges::all_of(calls, [](unsigned char x) { return x == 1; }));

        const int max_side = std::min(height, width);
        std::vector<std::uint64_t> by_side(static_cast<std::size_t>(max_side + 1));
        std::optional<BinaryGridSquare> largest;
        std::uint64_t total = 0;
        for (int top = 0; top < height; ++top)
        {
            for (int left = 0; left < width; ++left)
            {
                for (int side = 1; top + side <= height && left + side <= width; ++side)
                {
                    bool filled = true;
                    for (int i = top; i < top + side; ++i)
                    {
                        for (int j = left; j < left + side; ++j)
                        {
                            filled = filled && good[position(i, j)] != 0;
                        }
                    }
                    if (!filled)
                    {
                        break;
                    }
                    ++total;
                    ++by_side[static_cast<std::size_t>(side)];
                    if (!largest || largest->side < side)
                    {
                        largest = BinaryGridSquare{top, left, side};
                    }
                }
            }
        }
        assert(actual.count == total && actual.by_side == by_side);
        if (largest)
        {
            expect_square(actual.largest, largest->top, largest->left, largest->side);
        }
        else
        {
            assert(!actual.largest);
        }
    }
}

int main()
{
    boundary_cases();
    random_cases();
}
