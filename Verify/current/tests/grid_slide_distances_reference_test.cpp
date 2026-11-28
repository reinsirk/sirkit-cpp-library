// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Grid_Slide_Distances.cpp"

#include <array>
#include <cassert>
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
using Axes = sirkit::GridSlideAxes;

std::vector<int> brute(const std::vector<std::string> &grid, Point start, Axes axes, char wall)
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
    const auto directions = axes == Axes::diagonal
                                ? std::array{Point{-1, -1}, Point{-1, 1}, Point{1, -1}, Point{1, 1}}
                                : std::array{Point{-1, 0}, Point{1, 0}, Point{0, -1}, Point{0, 1}};
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const auto [r, c] = pending[i];
        for (const auto &[dr, dc] : directions)
        {
            for (int x = r + dr, y = c + dc; 0 <= x && x < h && 0 <= y && y < w && grid[x][y] != wall; x += dr, y += dc)
            {
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

void check(const std::vector<std::string> &grid, Point start, Point target, Axes axes, char wall)
{
    const auto expected = brute(grid, start, axes, wall);
    assert(sirkit::grid_slide_distances(grid, start, axes, wall) == expected);
    assert(sirkit::grid_slide_distance(grid, start, target, axes, wall) ==
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
    check({"."}, {0, 0}, {0, 0}, Axes::diagonal, '#');
    check({"@"}, {0, 0}, {0, 0}, Axes::orthogonal, '@');
    check({".", ".", "."}, {0, 0}, {2, 0}, Axes::diagonal, '#');
    check({"...."}, {0, 0}, {0, 3}, Axes::orthogonal, '#');
    check({"..", ".#"}, {0, 0}, {1, 1}, Axes::orthogonal, '#');
    check({"..#.#"}, {0, 0}, {0, 3}, Axes::diagonal, '#');
    must_throw<std::invalid_argument>([]
                                      {
                                          static_cast<void>(sirkit::grid_slide_distances({}, {0, 0}));
                                      });
    must_throw<std::invalid_argument>([]
                                      {
                                          const std::vector<std::string> rows{""};
                                          static_cast<void>(sirkit::grid_slide_distances(rows, {0, 0}));
                                      });
    must_throw<std::invalid_argument>([]
                                      {
                                          const std::vector<std::string> grid{"..", "."};
                                          static_cast<void>(sirkit::grid_slide_distances(grid, {0, 0}));
                                      });
    must_throw<std::length_error>([]
                                  {
                                      static_cast<void>(sirkit::detail::grid_stride_area(3, std::numeric_limits<int>::max()));
                                  });
#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<std::string> grid{"..", ".."};
    for (const Point &bad : {Point{-1, 0}, Point{2, 0}, Point{0, -1}, Point{0, 2}})
    {
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(sirkit::grid_slide_distances(grid, bad));
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(sirkit::grid_slide_distance(grid, {0, 0}, bad));
                                      });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(246);
    for (int trial = 0; trial < 1450; ++trial)
    {
        const int h = 1 + static_cast<int>(rng() % 8);
        const int w = 1 + static_cast<int>(rng() % 8);
        std::vector<std::string> grid(h, std::string(w, '.'));
        const char wall = trial % 2 == 0 ? '#' : '@';
        for (auto &row : grid)
        {
            for (auto &cell : row)
            {
                if (rng() % 6 < static_cast<unsigned>(trial % 4))
                {
                    cell = wall;
                }
            }
        }
        const Point start{rng() % h, rng() % w};
        const Point target{rng() % h, rng() % w};
        check(grid, start, target, Axes::diagonal, wall);
        check(grid, start, target, Axes::orthogonal, wall);
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        static_cast<void>(sirkit::grid_slide_distances({}, {0, 0}));
        return 1;
    }
    check_limits();
    check_random();
}
