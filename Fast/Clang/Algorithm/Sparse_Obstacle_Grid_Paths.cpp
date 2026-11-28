// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Sparse_Obstacle_Grid_Paths.cpp
// 参考（疎な障害物の用途）: https://atcoder.jp/contests/dp/tasks/dp_y
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_obstacle_grid_paths.md
//
// API一覧
// count_sparse_grid_paths(height,width,walls,choose): 左上から右下へ、右・下移動で壁を避ける経路数。
// wallsは0始まりの(行,列)。順不同・重複可。空グリッドや両端のどちらかが壁なら0。
// choose(n,k)はint64_tの添字を受け、その返却型で二項係数を返す。演算は0の構築・乗算・減算。
// 壁の数Kに対してO((K+1)^2)時間・O(K+1)領域。二項係数と返却型の演算をO(1)とする。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SPARSE_OBSTACLE_GRID_PATHS_DEFINED
#define SIRKIT_SPARSE_OBSTACLE_GRID_PATHS_DEFINED

template <typename Choose>
[[nodiscard]] auto count_sparse_grid_paths(
    int height, int width, std::span<const std::pair<int, int>> walls, Choose choose)
    -> std::remove_cvref_t<std::invoke_result_t<Choose &, std::int64_t, std::int64_t>>
{
    using Ways = std::remove_cvref_t<std::invoke_result_t<Choose &, std::int64_t, std::int64_t>>;
    if (height == 0 || width == 0)
    {
        return Ways{0};
    }
    const std::pair start{0, 0};
    const std::pair finish{height - 1, width - 1};
    std::vector points(walls.begin(), walls.end());
    std::ranges::sort(points);
    points.erase(std::unique(points.begin(), points.end()), points.end());
    if (!points.empty() && (points.front() == start || points.back() == finish))
    {
        return Ways{0};
    }
    points.push_back(finish);

    std::vector<Ways> leftmost_hit;
    leftmost_hit.reserve(points.size());
    for (std::size_t i = 0; i < points.size(); ++i)
    {
        const auto [row, col] = points[i];
        Ways ways = std::invoke(choose, static_cast<std::int64_t>(row) + col, static_cast<std::int64_t>(row));
        // この点より先に通る壁ごとに、その壁を最初に踏む経路を引く。
        for (std::size_t j = 0; j < i; ++j)
        {
            const auto [prior_row, prior_col] = points[j];
            if (prior_col > col)
            {
                continue;
            }
            const auto dr = static_cast<std::int64_t>(row) - prior_row;
            const auto dc = static_cast<std::int64_t>(col) - prior_col;
            ways = ways - leftmost_hit[j] * std::invoke(choose, dr + dc, dr);
        }
        leftmost_hit.push_back(std::move(ways));
    }
    return std::move(leftmost_hit.back());
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
