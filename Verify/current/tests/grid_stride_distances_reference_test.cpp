// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Grid_Stride_Distances.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
using Point = std::pair<int, int>;

std::vector<int> brute(const std::vector<std::string> &grid, Point start, std::uint64_t k, char wall)
{
    const int h = static_cast<int>(grid.size());
    const int w = static_cast<int>(grid.front().size());
    std::vector<int> answer(h * w, -1);
    if (grid[start.first][start.second] == wall)
    {
        return answer;
    }
    std::vector<Point> pending{start};
    answer[start.first * w + start.second] = 0;
    constexpr std::array directions{Point{-1, 0}, Point{1, 0}, Point{0, -1}, Point{0, 1}};
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const auto [r, c] = pending[i];
        for (const auto &[dr, dc] : directions)
        {
            int x = r;
            int y = c;
            for (std::uint64_t step = 0; step < k; ++step)
            {
                x += dr;
                y += dc;
                if (x < 0 || x >= h || y < 0 || y >= w || grid[x][y] == wall)
                {
                    break;
                }
                if (answer[x * w + y] == -1)
                {
                    answer[x * w + y] = 1 + answer[r * w + c];
                    pending.emplace_back(x, y);
                }
            }
        }
    }
    return answer;
}

void check(const std::vector<std::string> &grid, Point start, Point target, std::uint64_t k, char wall)
{
    const auto expected = brute(grid, start, k, wall);
    assert(sirkit::grid_stride_distances(grid, start, k, wall) == expected);
    assert(sirkit::grid_stride_distance(grid, start, target, k, wall) ==
           expected[target.first * grid.front().size() + target.second]);
}

template <class Exception, class Operation>
void must_throw(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

void check_limits()
{
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    check({"."}, {0, 0}, {0, 0}, 1, '#');
    check({"@"}, {0, 0}, {0, 0}, 1, '@');
    check({".", ".", "."}, {0, 0}, {2, 0}, kMax, '#');
    check({"...."}, {0, 0}, {0, 3}, kMax, '#');
    check({"..."}, {0, 0}, {0, 2}, 0, '#');
    check({"..", ".#"}, {0, 0}, {1, 1}, 1, '#');
    check({"..#.#"}, {0, 0}, {0, 3}, 10, '#');
    must_throw<std::invalid_argument>([]
                                      {
                                          static_cast<void>(sirkit::grid_stride_distances({}, {0, 0}, 1));
                                      });
    must_throw<std::invalid_argument>([]
                                      {
                                          const std::vector<std::string> rows{""};
                                          static_cast<void>(sirkit::grid_stride_distances(rows, {0, 0}, 1));
                                      });
    must_throw<std::invalid_argument>([]
                                      {
                                          const std::vector<std::string> grid{"..", "."};
                                          static_cast<void>(sirkit::grid_stride_distances(grid, {0, 0}, 1));
                                      });
    must_throw<std::length_error>([]
                                  {
                                      static_cast<void>(sirkit::detail::grid_stride_area(2, std::numeric_limits<int>::max()));
                                  });
#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<std::string> grid{"..", ".."};
    for (const Point &bad : {Point{-1, 0}, Point{2, 0}, Point{0, -1}, Point{0, 2}})
    {
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(sirkit::grid_stride_distances(grid, bad, 1));
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(sirkit::grid_stride_distance(grid, {0, 0}, bad, 1));
                                      });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(170);
    for (int trial = 0; trial < 1600; ++trial)
    {
        const int h = 1 + static_cast<int>(rng() % 8);
        const int w = 1 + static_cast<int>(rng() % 8);
        std::vector<std::string> grid(h, std::string(w, '.'));
        const char wall = trial % 2 == 0 ? '#' : '@';
        for (auto &row : grid)
        {
            for (auto &cell : row)
            {
                if (rng() % 5 == 0)
                {
                    cell = wall;
                }
            }
        }
        const Point start{rng() % h, rng() % w};
        const Point target{rng() % h, rng() % w};
        check(grid, start, target, rng() % 11, wall);
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        static_cast<void>(sirkit::grid_stride_distances({}, {0, 0}, 1));
        return 1;
    }
    check_limits();
    check_random();
}
