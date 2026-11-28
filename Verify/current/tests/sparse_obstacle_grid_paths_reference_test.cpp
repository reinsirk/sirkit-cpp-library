// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Sparse_Obstacle_Grid_Paths.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using sirkit::count_sparse_grid_paths;
using Point = std::pair<int, int>;

std::int64_t choose_small(std::int64_t n, std::int64_t k)
{
    std::int64_t result = 1;
    for (std::int64_t i = 1; i <= k; ++i)
    {
        result = result * (n - i + 1) / i;
    }
    return result;
}

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

void edge_cases()
{
    const auto unused = [](std::int64_t, std::int64_t)
    {
        assert(false);
        return std::int64_t{0};
    };
    assert(count_sparse_grid_paths(0, 3, {}, unused) == 0);
    assert(count_sparse_grid_paths(3, 0, {}, unused) == 0);
    assert(count_sparse_grid_paths(1, 1, {}, choose_small) == 1);
    const std::array start{Point{0, 0}}, finish{Point{3, 4}};
    assert(count_sparse_grid_paths(4, 5, start, unused) == 0);
    assert(count_sparse_grid_paths(4, 5, finish, unused) == 0);
    const std::array one_wall_repeated{Point{1, 1}, Point{1, 1}, Point{1, 1}};
    assert(count_sparse_grid_paths(3, 3, one_wall_repeated, choose_small) == 2);
    const std::array incomparable{Point{2, 1}, Point{1, 2}};
    assert(count_sparse_grid_paths(3, 3, incomparable, choose_small) == 0);

    constexpr int limit = std::numeric_limits<int>::max();
    const auto wide = count_sparse_grid_paths(limit, limit, {}, [](std::int64_t n, std::int64_t k)
    {
        assert(k == limit - 1 && n == 2 * static_cast<std::int64_t>(limit - 1));
        return 17;
    });
    assert(wide == 17);

#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::invalid_argument>([&]
    {
        static_cast<void>(count_sparse_grid_paths(-1, 0, {}, unused));
    });
    expect_error<std::invalid_argument>([&]
    {
        static_cast<void>(count_sparse_grid_paths(0, -1, {}, unused));
    });
    for (const Point &point : {Point{-1, 0}, Point{4, 0}, Point{0, -1}, Point{0, 5}})
    {
        const std::array invalid{point};
        expect_error<std::out_of_range>([&]
        {
            static_cast<void>(count_sparse_grid_paths(4, 5, invalid, unused));
        });
    }
#endif
}

void random_small_grids()
{
    std::mt19937 random(0xc7521);
    for (int test = 0; test < 2000; ++test)
    {
        const int h = 1 + static_cast<int>(random() % 9), w = 1 + static_cast<int>(random() % 9);
        std::vector<Point> walls;
        std::vector<unsigned char> blocked(static_cast<std::size_t>(h * w));
        std::vector<std::int64_t> values(blocked.size());
        const auto index = [w](int i, int j)
        {
            return static_cast<std::size_t>(i * w + j);
        };
        for (std::size_t i = 0, k = random() % 15; i < k; ++i)
        {
            const int row = static_cast<int>(random() % h), col = static_cast<int>(random() % w);
            walls.emplace_back(row, col);
            blocked[index(row, col)] = 1;
        }
        if (!blocked[0])
        {
            values[0] = 1;
        }
        for (int i = 0; i < h; ++i)
        {
            for (int j = 0; j < w; ++j)
            {
                if (blocked[index(i, j)])
                {
                    continue;
                }
                if (i != 0)
                {
                    values[index(i, j)] += values[index(i - 1, j)];
                }
                if (j != 0)
                {
                    values[index(i, j)] += values[index(i, j - 1)];
                }
            }
        }
        assert(count_sparse_grid_paths(h, w, walls, choose_small) == values.back());
    }
}

int main()
{
    edge_cases();
    random_small_grids();
}
