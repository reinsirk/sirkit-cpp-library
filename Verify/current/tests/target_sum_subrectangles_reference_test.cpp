// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Target_Sum_Subrectangles.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

using sirkit::count_subrectangles_with_sum;
using Value = std::int64_t;
using Count = std::uint64_t;

#ifndef SKIP_RUNTIME_CHECKS
template <typename Error, typename Operation>
void expect_error(Operation operation)
{
    bool rejected = false;
    try
    {
        operation();
    }
    catch (const Error &)
    {
        rejected = true;
    }
    assert(rejected);
}
#endif

void boundary_cases()
{
    const auto unused = [](int, int)
    {
        assert(false);
        return 0;
    };
    assert(count_subrectangles_with_sum(0, 100, 0, unused) == 0);
    assert(count_subrectangles_with_sum(100, 0, 10, unused) == 0);

    constexpr Value low = std::numeric_limits<Value>::min();
    constexpr Value high = std::numeric_limits<Value>::max();
    const auto low_cell = [](int, int)
    {
        return low;
    };
    const auto high_cell = [](int, int)
    {
        return high;
    };
    assert(count_subrectangles_with_sum(1, 1, low, high_cell) == 0);
    assert(count_subrectangles_with_sum(1, 1, high, low_cell) == 0);
    assert(count_subrectangles_with_sum(1, 1, low, low_cell) == 1);
    assert(count_subrectangles_with_sum(1, 1, high, high_cell) == 1);

    const std::array<Value, 3> values{high, low, 1};
    const auto row = [&](int, int col)
    {
        return values[static_cast<std::size_t>(col)];
    };
    assert(count_subrectangles_with_sum(1, 3, 0, row) == 1);
    assert(count_subrectangles_with_sum(1, 3, -1, row) == 1);
    assert(count_subrectangles_with_sum(1, 3, 1, row) == 1);
    assert(count_subrectangles_with_sum(1, 3, low + 1, row) == 1);

    constexpr Count max_count = std::numeric_limits<Count>::max();
    assert(sirkit::detail::AddSubrectangleCount(max_count - 1, 1) == max_count);
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::invalid_argument>([&] { static_cast<void>(count_subrectangles_with_sum(-1, 1, 0, unused)); });
    expect_error<std::invalid_argument>([&] { static_cast<void>(count_subrectangles_with_sum(1, -1, 0, unused)); });
    expect_error<std::overflow_error>([]
    {
        static_cast<void>(count_subrectangles_with_sum(1, 2, 0, [](int, int col)
        {
            return col == 0 ? high : 1;
        }));
    });
    expect_error<std::overflow_error>([]
    {
        static_cast<void>(count_subrectangles_with_sum(1, 2, 0, [](int, int col)
        {
            return col == 0 ? low : -1;
        }));
    });
    expect_error<std::overflow_error>([]
    {
        static_cast<void>(sirkit::detail::AddSubrectangleCount(max_count, 1));
    });
#endif
}

void random_matrices()
{
    std::mt19937 random(0xab7410);
    for (int test = 0; test < 1800; ++test)
    {
        const int h = static_cast<int>(random() % 7);
        const int w = static_cast<int>(random() % 7);
        const Value target = static_cast<Value>(random() % 29) - 14;
        std::vector<Value> grid(static_cast<std::size_t>(h * w));
        for (auto &value : grid)
        {
            value = static_cast<Value>(random() % 15) - 7;
        }
        const auto at = [&](int row, int col)
        {
            assert(0 <= row && row < h && 0 <= col && col < w);
            return grid[static_cast<std::size_t>(row * w + col)];
        };

        Count expected = 0;
        for (int top = 0; top < h; ++top)
        {
            for (int bottom = top + 1; bottom <= h; ++bottom)
            {
                for (int left = 0; left < w; ++left)
                {
                    for (int right = left + 1; right <= w; ++right)
                    {
                        Value sum = 0;
                        for (int row = top; row < bottom; ++row)
                        {
                            for (int col = left; col < right; ++col)
                            {
                                sum += at(row, col);
                            }
                        }
                        expected += sum == target;
                    }
                }
            }
        }
        assert(count_subrectangles_with_sum(h, w, target, at) == expected);
    }
}

int main()
{
    boundary_cases();
    random_matrices();
}
