// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Count_Points_In_Triangle.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/count_points_in_triangle.md
//
// API一覧
// CountPointsInTriangle(vertices, points): 頂点候補N点と、数える対象M点から前処理する。
// count(a, b, c): 指定した0始まり添字3つの三角形の内部にある対象点数を返す。境界を除く。
// 頂点の向きは任意。面積0なら0。同じ座標の対象点は重複も数える。
// 2集合を合わせた各軸の座標幅は2×10^9以下。範囲外はoverflow_error。
// N>0で前処理O(N²(M+1))時間、O(N²)追加領域。countはO(1)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COUNT_POINTS_IN_TRIANGLE_DEFINED
#define SIRKIT_COUNT_POINTS_IN_TRIANGLE_DEFINED

class CountPointsInTriangle
{
public:
    using Point = std::pair<std::int64_t, std::int64_t>;

    CountPointsInTriangle(std::span<const Point> vertices, std::span<const Point> points)
        : vertices_(vertices.begin(), vertices.end()), multiplicity_(vertices.size())
    {
        if (vertices.empty())
        {
            return;
        }
        CheckBounds(vertices, points);
        scores_.assign(vertices.size(), std::vector<std::int64_t>(vertices.size()));
        for (std::size_t i = 0; i < vertices_.size(); ++i)
        {
            multiplicity_[i] = std::ranges::count(points, vertices_[i]);
            for (std::size_t j = 0; j < i; ++j)
            {
                BuildEdge(i, j, points);
            }
        }
    }

    std::size_t count(std::size_t a, std::size_t b, std::size_t c) const
    {
        const auto area = Side(vertices_[a], vertices_[b], vertices_[c]);
        if (area == 0)
        {
            return 0;
        }
        if (area < 0)
        {
            std::swap(b, c);
        }
        const bool ab = ShiftInward(vertices_[a], vertices_[b]);
        const bool bc = ShiftInward(vertices_[b], vertices_[c]);
        const bool ca = ShiftInward(vertices_[c], vertices_[a]);

        auto result = scores_[a][b] + scores_[b][c] + scores_[c][a];
        // 頂点上の点が内側へ移るのは、接する2辺がどちらも内向きの場合だけ。
        result -= multiplicity_[a] * (ca && ab);
        result -= multiplicity_[b] * (ab && bc);
        result -= multiplicity_[c] * (bc && ca);
        return static_cast<std::size_t>(result);
    }

private:
    std::vector<Point> vertices_;
    std::vector<std::int64_t> multiplicity_;
    std::vector<std::vector<std::int64_t>> scores_;

    static void CheckBounds(std::span<const Point> vertices, std::span<const Point> points)
    {
        Point low = vertices.front(), high = low;
        for (const auto set : {vertices, points})
        {
            for (const auto &[x, y] : set)
            {
                low.first = std::min(low.first, x);
                low.second = std::min(low.second, y);
                high.first = std::max(high.first, x);
                high.second = std::max(high.second, y);
            }
        }
        constexpr std::uint64_t kMaxSpan = 2'000'000'000;
        const auto width = [](std::int64_t min, std::int64_t max)
        {
            return static_cast<std::uint64_t>(max) - static_cast<std::uint64_t>(min);
        };
        if (width(low.first, high.first) > kMaxSpan || width(low.second, high.second) > kMaxSpan)
        {
            throw std::overflow_error("CountPointsInTriangle: coordinate range is too wide");
        }
    }

    static std::int64_t Side(const Point &a, const Point &b, const Point &p)
    {
        return (b.first - a.first) * (p.second - a.second) - (b.second - a.second) * (p.first - a.first);
    }

    static bool ShiftInward(const Point &a, const Point &b)
    {
        // 評価点を(ε, ε²)だけ動かすとき、有向辺の左側へ入るか。
        return b.second < a.second || (a.second == b.second && a.first < b.first);
    }

    void BuildEdge(std::size_t a, std::size_t b, std::span<const Point> points)
    {
        const auto order = [](const Point &point)
        {
            return std::pair(point.second, point.first);
        };
        if (order(vertices_[b]) < order(vertices_[a]))
        {
            std::swap(a, b);
        }
        const auto &lower = vertices_[a], &upper = vertices_[b];
        if (lower.second == upper.second)
        {
            // 水平な辺は半直線と交差しない。右向きの辺からはずらして数えた辺上の点を引く。
            for (const auto &[x, y] : points)
            {
                scores_[a][b] -= y == lower.second && lower.first < x && x < upper.first;
            }
            return;
        }
        std::int64_t crossings = 0, border = 0;
        for (const auto &point : points)
        {
            if (lower.second <= point.second && point.second < upper.second)
            {
                const auto side = Side(lower, upper, point);
                crossings += side > 0;
                border += side == 0 && lower.second < point.second;
            }
        }
        // (ε, ε²)のずれを入れた巻き数。下向きの辺では端点以外の辺上の点を補正する。
        scores_[a][b] = crossings;
        scores_[b][a] = -crossings - border;
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
