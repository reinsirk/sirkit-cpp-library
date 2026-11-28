// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Largest_Rectangle.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

static void check_histogram(const std::vector<std::uint64_t> &heights)
{
    std::uint64_t expected = 0;
    for (std::size_t left = 0; left < heights.size(); ++left)
    {
        auto minimum = heights[left];
        for (auto right = left; right < heights.size(); ++right)
        {
            minimum = std::min(minimum, heights[right]);
            expected = std::max(expected, minimum * (right - left + 1));
        }
    }
    const auto answer = sirkit::largest_histogram_rectangle(heights);
    assert(answer.area == expected);
    if (answer.area == 0)
    {
        assert(answer.left == 0 && answer.right == 0 && answer.height == 0);
        return;
    }
    assert(answer.left < answer.right && answer.right <= heights.size());
    assert(answer.area == answer.height * (answer.right - answer.left));
    for (auto i = answer.left; i < answer.right; ++i)
    {
        assert(heights[i] >= answer.height);
    }
}

static void check_grid(std::size_t rows, std::size_t cols, const std::vector<bool> &cells)
{
    std::uint64_t expected = 0;
    for (std::size_t top = 0; top < rows; ++top)
    {
        for (std::size_t left = 0; left < cols; ++left)
        {
            for (auto bottom = top + 1; bottom <= rows; ++bottom)
            {
                for (auto right = left + 1; right <= cols; ++right)
                {
                    bool valid = true;
                    for (auto row = top; row < bottom; ++row)
                    {
                        for (auto col = left; col < right; ++col)
                        {
                            valid &= cells[row * cols + col];
                        }
                    }
                    if (valid)
                    {
                        expected = std::max<std::uint64_t>(expected, (bottom - top) * (right - left));
                    }
                }
            }
        }
    }
    std::size_t next = 0;
    const auto answer = sirkit::largest_grid_rectangle(rows, cols, [&](std::size_t row, std::size_t col)
                                                       {
                                                           assert(next == row * cols + col);
                                                           return cells[next++];
                                                       });
    assert(next == rows * cols && answer.area == expected);
    if (expected == 0)
    {
        assert(answer.top == 0 && answer.bottom == 0 && answer.left == 0 && answer.right == 0);
        return;
    }
    assert(answer.top < answer.bottom && answer.bottom <= rows && answer.left < answer.right && answer.right <= cols);
    assert(answer.area == (answer.bottom - answer.top) * (answer.right - answer.left));
    for (auto row = answer.top; row < answer.bottom; ++row)
    {
        for (auto col = answer.left; col < answer.right; ++col)
        {
            assert(cells[row * cols + col]);
        }
    }
}

int main()
{
    check_histogram({});
    check_histogram({0, 0, 0});
    check_histogram({3, 3, 3, 2, 1});
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    check_histogram({kMax});
    bool caught = false;
    try
    {
        const std::array too_large{kMax, kMax};
        (void)sirkit::largest_histogram_rectangle(too_large);
    }
    catch (const std::overflow_error &)
    {
        caught = true;
    }
    assert(caught);

    check_grid(0, 0, {});
    check_grid(0, 4, {});
    check_grid(5, 0, {});
    check_grid(2, 3, {false, false, false, false, false, false});
    check_grid(2, 3, {true, true, true, true, true, true});
    std::mt19937 rng(25436);
    for (std::size_t i = 0; i < 600; ++i)
    {
        std::vector<std::uint64_t> heights(rng() % 38);
        for (auto &height : heights)
        {
            height = rng() % 15;
        }
        check_histogram(heights);
        const std::size_t rows = rng() % 7, cols = rng() % 7;
        std::vector<bool> cells(rows * cols);
        for (std::size_t k = 0; k < cells.size(); ++k)
        {
            cells[k] = (rng() % 3) != 0;
        }
        check_grid(rows, cols, cells);
    }
    constexpr std::size_t kSize = 1400;
    const auto full = sirkit::largest_grid_rectangle(kSize, kSize, [](std::size_t, std::size_t)
                                                     {
                                                         return true;
                                                     });
    assert(full.area == kSize * kSize);
    const auto staggered = sirkit::largest_grid_rectangle(kSize, kSize, [](std::size_t row, std::size_t col)
                                                          {
                                                              return (row + col) % 2 == 0;
                                                          });
    assert(staggered.area == 1);
}
