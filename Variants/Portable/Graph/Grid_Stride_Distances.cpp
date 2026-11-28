// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Grid_Stride_Distances.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/grid_stride_distances.md
//
// API一覧
// grid_stride_distances(grid,start,max_stride,wall='#'): 始点から各マスへ最少何回で移動できるか。
// grid_stride_distance(grid,start,target,max_stride,wall='#'): 指定した1マスへの最少回数。
//   座標は0始まりの(行,列)。上下左右へ1..max_strideマス直進し、壁を通らない。
//   壁または到達不能の距離は-1。全距離の添字はrow*width+column。
//   O(HW α(HW))時間、O(HW)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GRID_STRIDE_LAYOUT_DEFINED
#define SIRKIT_GRID_STRIDE_LAYOUT_DEFINED

namespace detail
{
inline int grid_stride_area(std::size_t height, std::size_t width)
{
    if (height == 0 || width == 0)
    {
        throw std::invalid_argument("grid stride: empty grid");
    }
    constexpr auto kMax = static_cast<std::size_t>(std::numeric_limits<int>::max() - 1);
    if (height > kMax / width)
    {
        throw std::length_error("grid stride: too many cells");
    }
    return static_cast<int>(height * width);
}

struct StrideGrid
{
    std::span<const std::string> rows;
    int height, width, area;
    char wall;

    [[nodiscard]] int id(std::pair<int, int> point) const
    {
        const auto [r, c] = point;
#ifndef SKIP_RUNTIME_CHECKS
        if (r < 0 || r >= height || c < 0 || c >= width)
        {
            throw std::out_of_range("grid stride: coordinate out of range");
        }
#endif
        return r * width + c;
    }
};

inline StrideGrid make_stride_grid(std::span<const std::string> rows, char wall)
{
    const std::size_t width = rows.empty() ? 0 : rows.front().size();
    const int area = grid_stride_area(rows.size(), width);
    if (std::ranges::any_of(rows, [width](const std::string &row)
                            {
                                return row.size() != width;
                            }))
    {
        throw std::invalid_argument("grid stride: nonrectangular grid");
    }
    return {rows, static_cast<int>(rows.size()), static_cast<int>(width), area, wall};
}
} // namespace detail

#endif

#ifndef SIRKIT_GRID_STRIDE_DISTANCES_DEFINED
#define SIRKIT_GRID_STRIDE_DISTANCES_DEFINED

namespace detail
{
class GridStrideSuccessors
{
public:
    explicit GridStrideSuccessors(int size) : parent_(size + 1, -1), largest_(size + 1)
    {
        std::iota(largest_.begin(), largest_.end(), 0);
    }

    int next(int position)
    {
        return largest_[root(position)];
    }

    void erase(int position)
    {
        int a = root(position);
        int b = root(position + 1);
        if (parent_[a] > parent_[b])
        {
            std::swap(a, b);
        }
        parent_[a] += parent_[b];
        parent_[b] = a;
        largest_[a] = std::max(largest_[a], largest_[b]);
    }

private:
    int root(int position)
    {
        int leader = position;
        while (parent_[leader] >= 0)
        {
            leader = parent_[leader];
        }
        while (position != leader)
        {
            const int next = parent_[position];
            parent_[position] = leader;
            position = next;
        }
        return leader;
    }

    std::vector<int> parent_;
    std::vector<int> largest_;
};

template <class IsOpen, class FlatIndex>
std::vector<std::pair<int, int>> grid_stride_intervals(int lines, int length, IsOpen is_open, FlatIndex index)
{
    std::vector<std::pair<int, int>> interval(static_cast<std::size_t>(lines) * length);
    for (int line = 0; line < lines; ++line)
    {
        for (int first = 0; first < length;)
        {
            if (!is_open(line, first))
            {
                ++first;
                continue;
            }
            int last = first + 1;
            while (last < length && is_open(line, last))
            {
                ++last;
            }
            for (int step = first; step < last; ++step)
            {
                interval[index(line, step)] = {first, last - 1};
            }
            first = last;
        }
    }
    return interval;
}

inline std::vector<int> grid_stride_search(const StrideGrid &grid, int source, int target, std::uint64_t max_stride)
{
    const auto [rows, height, width, area, wall] = grid;
    const auto is_open = [&](int r, int c)
    {
        return rows[r][c] != wall;
    };
    const auto blocked = [&](int id)
    {
        return !is_open(id / width, id % width);
    };
    std::vector<int> distance(area, -1);
    if (blocked(source) || (target >= 0 && blocked(target)))
    {
        return distance;
    }
    distance[source] = 0;
    if (source == target || max_stride == 0)
    {
        return distance;
    }

    const auto horizontal = grid_stride_intervals(height, width, is_open,
                                                  [width](int r, int c)
                                                  {
                                                      return r * width + c;
                                                  });
    const auto vertical = grid_stride_intervals(width, height, [&](int c, int r)
                                                {
                                                    return is_open(r, c);
                                                },
                                                [width](int c, int r)
                                                {
                                                    return r * width + c;
                                                });
    const int move_x = static_cast<int>(std::min<std::uint64_t>(max_stride, width - 1));
    const int move_y = static_cast<int>(std::min<std::uint64_t>(max_stride, height - 1));

    GridStrideSuccessors by_row(area), by_column(area);
    std::vector<int> queue;
    queue.reserve(area);
    auto discover = [&](int cell, int d)
    {
        distance[cell] = d;
        queue.push_back(cell);
        by_row.erase(cell);
        by_column.erase((cell % width) * height + cell / width);
    };
    discover(source, 0);
    for (std::size_t i = 0; i < queue.size(); ++i)
    {
        const int u = queue[i];
        if (u == target)
        {
            break;
        }
        const int r = u / width;
        const int c = u % width;
        const int after = distance[u] + 1;
        const auto [left_wall, right_wall] = horizontal[u];
        const auto [top_wall, bottom_wall] = vertical[u];
        const int left = std::max(left_wall, c - move_x);
        const int right = std::min(right_wall, c + std::min(move_x, width - 1 - c));
        const int top = std::max(top_wall, r - move_y);
        const int bottom = std::min(bottom_wall, r + std::min(move_y, height - 1 - r));
        const int row_start = r * width;
        const int column_start = c * height;
        for (int flat = by_row.next(row_start + left); flat <= row_start + right; flat = by_row.next(flat))
        {
            discover(flat, after);
        }
        for (int flat = by_column.next(column_start + top); flat <= column_start + bottom; flat = by_column.next(flat))
        {
            discover((flat - column_start) * width + c, after);
        }
    }
    return distance;
}
} // namespace detail

inline std::vector<int> grid_stride_distances(std::span<const std::string> grid, std::pair<int, int> start,
                                              std::uint64_t max_stride, char wall = '#')
{
    const auto view = detail::make_stride_grid(grid, wall);
    return detail::grid_stride_search(view, view.id(start), -1, max_stride);
}

inline int grid_stride_distance(std::span<const std::string> grid, std::pair<int, int> start,
                                std::pair<int, int> target, std::uint64_t max_stride, char wall = '#')
{
    const auto view = detail::make_stride_grid(grid, wall);
    const int goal = view.id(target);
    return detail::grid_stride_search(view, view.id(start), goal, max_stride)[goal];
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
