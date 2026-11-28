// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Grid_Slide_Distances.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/grid_slide_distances.md
//
// API一覧
// grid_slide_distances(grid,start,axes=GridSlideAxes::orthogonal,wall='#'): 全マスまでの最少手数。
// grid_slide_distance(grid,start,target,axes=GridSlideAxes::orthogonal,wall='#'): 指定したマスまでの最少手数。
//   各手で壁に当たる手前まで、同じ方向へ任意の正のマス数だけ進める。
//   orthogonalは縦横、diagonalは斜め。座標は0始まりの(行,列)。
//   壁や到達不能なら-1。全距離の添字はrow*width+column。O(HW)時間・領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
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

#ifndef SIRKIT_GRID_SLIDE_DISTANCES_DEFINED
#define SIRKIT_GRID_SLIDE_DISTANCES_DEFINED

enum class GridSlideAxes
{
    orthogonal,
    diagonal,
};

namespace detail
{
inline std::vector<int> grid_slide_search(const StrideGrid &grid, int source, int target, GridSlideAxes axes)
{
    const auto [rows, height, width, area, wall] = grid;
    const auto blocked = [&](int id)
    {
        return rows[id / width][id % width] == wall;
    };
    std::vector<int> result(area, -1);
    if (blocked(source) || (target >= 0 && blocked(target)))
    {
        return result;
    }
    result[source] = 0;
    if (source == target)
    {
        return result;
    }

    using Point = std::pair<int, int>;
    const auto directions = axes == GridSlideAxes::diagonal
                                ? std::array{Point{1, 1}, Point{1, -1}}
                                : std::array{Point{0, 1}, Point{1, 0}};
    struct Segment
    {
        int start, length;
    };
    std::array<std::vector<Segment>, 2> segments;
    std::vector<std::array<int, 2>> label(area, {-1, -1});
    for (int r = 0; r < height; ++r)
    {
        for (int c = 0; c < width; ++c)
        {
            if (rows[r][c] == wall)
            {
                continue;
            }
            const int cell = r * width + c;
            for (int i = 0; i < 2; ++i)
            {
                const auto [dr, dc] = directions[i];
                const int col = c - dc;
                int id = -1;
                if (r >= dr && 0 <= col && col < width)
                {
                    id = label[(r - dr) * width + col][i];
                }
                if (id == -1)
                {
                    id = static_cast<int>(segments[i].size());
                    segments[i].push_back({cell, 0});
                }
                label[cell][i] = id;
                ++segments[i][id].length;
            }
        }
    }

    std::array distance{std::vector<int>(segments[0].size(), -1), std::vector<int>(segments[1].size(), -1)};
    std::vector<Point> pending;
    for (int axis = 0; axis < 2; ++axis)
    {
        const int id = label[source][axis];
        distance[axis][id] = 1;
        pending.emplace_back(axis, id);
    }
    for (std::size_t i = 0; i < pending.size(); ++i)
    {
        const auto [axis, id] = pending[i];
        const int depth = distance[axis][id];
        if (target >= 0 && label[target][axis] == id)
        {
            result[target] = depth;
            return result;
        }
        const int other = axis ^ 1;
        const auto [start, length] = segments[axis][id];
        const auto [dr, dc] = directions[axis];
        const int stride = dr * width + dc;
        for (int step = 0; step < length; ++step)
        {
            const int cell = start + step * stride;
            const int next = label[cell][other];
            if (distance[other][next] == -1)
            {
                distance[other][next] = depth + 1;
                pending.emplace_back(other, next);
            }
        }
    }
    // Every open cell touches two segments, so either both of them are reachable or neither is.
    for (int i = 0; i < area; ++i)
    {
        const auto [first, second] = label[i];
        if (i != source && first != -1)
        {
            result[i] = std::min(distance[0][first], distance[1][second]);
        }
    }
    return result;
}
} // namespace detail

inline std::vector<int> grid_slide_distances(std::span<const std::string> grid, std::pair<int, int> start,
                                             GridSlideAxes axes = GridSlideAxes::orthogonal, char wall = '#')
{
    const auto view = detail::make_stride_grid(grid, wall);
    return detail::grid_slide_search(view, view.id(start), -1, axes);
}

inline int grid_slide_distance(std::span<const std::string> grid, std::pair<int, int> start,
                               std::pair<int, int> target, GridSlideAxes axes = GridSlideAxes::orthogonal,
                               char wall = '#')
{
    const auto view = detail::make_stride_grid(grid, wall);
    const int goal = view.id(target);
    return detail::grid_slide_search(view, view.id(start), goal, axes)[goal];
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
