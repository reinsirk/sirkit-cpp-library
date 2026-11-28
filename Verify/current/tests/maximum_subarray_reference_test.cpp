// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Maximum_Subarray.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <ranges>
#include <vector>

static void check_subarray(const std::vector<int> &values)
{
    const auto best = sirkit::maximum_subarray_sum(values);
    if (values.empty())
    {
        assert(!best);
        return;
    }
    std::int64_t expected = std::numeric_limits<std::int64_t>::min();
    for (std::size_t left = 0; left < values.size(); ++left)
    {
        std::int64_t sum = 0;
        for (std::size_t right = left; right < values.size(); ++right)
        {
            sum += values[right];
            expected = std::max(expected, sum);
        }
    }
    assert(best && best->left < best->right && best->right <= values.size() && best->sum == expected);
    assert(std::accumulate(values.begin() + static_cast<std::ptrdiff_t>(best->left),
                           values.begin() + static_cast<std::ptrdiff_t>(best->right),
                           std::int64_t{0}) == expected);
}

static void check_submatrix(std::size_t rows, std::size_t columns, const std::vector<int> &values)
{
    const auto value_at = [&](std::size_t row, std::size_t column)
    {
        return values[row * columns + column];
    };
    const auto best = sirkit::maximum_submatrix_sum(rows, columns, value_at);
    if (values.empty())
    {
        assert(!best);
        return;
    }
    auto sum_rectangle = [&](std::size_t top, std::size_t bottom, std::size_t left, std::size_t right)
    {
        std::int64_t sum = 0;
        for (auto row = top; row < bottom; ++row)
        {
            for (auto column = left; column < right; ++column)
            {
                sum += value_at(row, column);
            }
        }
        return sum;
    };
    std::int64_t expected = std::numeric_limits<std::int64_t>::min();
    for (std::size_t top = 0; top < rows; ++top)
    {
        for (std::size_t bottom = top + 1; bottom <= rows; ++bottom)
        {
            for (std::size_t left = 0; left < columns; ++left)
            {
                for (std::size_t right = left + 1; right <= columns; ++right)
                {
                    expected = std::max(expected, sum_rectangle(top, bottom, left, right));
                }
            }
        }
    }
    assert(best && best->top < best->bottom && best->bottom <= rows);
    assert(best->left < best->right && best->right <= columns && best->sum == expected);
    assert(sum_rectangle(best->top, best->bottom, best->left, best->right) == expected);
}

int main()
{
    check_subarray({});
    check_subarray({-1, -2, -3});
    check_subarray({-7, 5, 0, -2, 3, -8});
    check_subarray({std::numeric_limits<int>::max(), std::numeric_limits<int>::max()});
    check_submatrix(0, 7, {});
    check_submatrix(4, 0, {});
    check_submatrix(2, 2, {-9, -2, -5, -3});
    check_submatrix(1, 2, {std::numeric_limits<int>::max(), std::numeric_limits<int>::max()});
    check_submatrix(3, 2, {-8, -7, 2, 3, 5, 7});
    const auto counter = std::views::iota(1, 1000);
    const auto best = sirkit::maximum_subarray_sum<std::uint64_t>(counter);
    assert(best && best->sum == 999 * 1000 / 2 && best->left == 0 && best->right == 999);

    std::mt19937 rng(320129);
    for (int trial = 0; trial < 350; ++trial)
    {
        const std::size_t rows = rng() % 8, columns = rng() % 8;
        std::vector<int> values(rows * columns);
        for (auto &value : values)
        {
            value = static_cast<int>(rng() % 30) - 15;
        }
        check_subarray(values);
        check_submatrix(rows, columns, values);
    }
}
