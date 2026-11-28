// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Lattice_Shape_Transforms.cpp
// 参考（格子図形の回転と平行移動）: https://atcoder.jp/contests/abc218/tasks/abc218_c
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lattice_shape_transforms.md
//
// API一覧
// lattice_shape_transforms(source,target,allow_reflection=false): 平行移動でtargetと一致するsourceの向きを列挙。
// LatticeOrientation: 恒等・90度単位の回転。反転ありの向きは第1座標を反転してから回転する。
// 点はint64_tの(x,y)の組。順不同、同一点の重複も照合する。空同士なら許されたすべての向き。
// n点が同数ならO(1+n*log(n+1))時間・O(n+1)領域。点数が異なれば空結果をO(1)で返す。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LATTICE_SHAPE_TRANSFORMS_DEFINED
#define SIRKIT_LATTICE_SHAPE_TRANSFORMS_DEFINED

enum class LatticeOrientation
{
    kIdentity,
    kRotate90,
    kRotate180,
    kRotate270,
    kReflect,
    kReflectThenRotate90,
    kReflectThenRotate180,
    kReflectThenRotate270
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
using LatticePoint = std::pair<std::int64_t, std::int64_t>;
using TranslatedLatticePoint = std::pair<std::uint64_t, std::uint64_t>;

struct NormalizedLatticeShape
{
    std::uint64_t width{};
    std::uint64_t height{};
    std::vector<TranslatedLatticePoint> points;
};

inline std::uint64_t LatticeCoordinateDistance(std::int64_t upper, std::int64_t lower) noexcept
{
    return static_cast<std::uint64_t>(upper) - static_cast<std::uint64_t>(lower);
}

inline NormalizedLatticeShape NormalizeLatticeShape(std::span<const LatticePoint> points)
{
    if (points.empty())
    {
        return {};
    }
    auto low = points.front();
    auto high = low;
    for (const auto &[x, y] : points)
    {
        low.first = std::min(low.first, x);
        low.second = std::min(low.second, y);
        high.first = std::max(high.first, x);
        high.second = std::max(high.second, y);
    }
    NormalizedLatticeShape shape{
        LatticeCoordinateDistance(high.first, low.first),
        LatticeCoordinateDistance(high.second, low.second),
        {}
    };
    shape.points.reserve(points.size());
    for (const auto &[x, y] : points)
    {
        shape.points.emplace_back(LatticeCoordinateDistance(x, low.first), LatticeCoordinateDistance(y, low.second));
    }
    return shape;
}

inline TranslatedLatticePoint ReorientLatticePoint(
    TranslatedLatticePoint point, std::uint64_t width, std::uint64_t height, int orientation) noexcept
{
    auto [x, y] = point;
    if (orientation >= 4)
    {
        x = width - x;
    }
    switch (orientation % 4)
    {
    case 0:
        return {x, y};
    case 1:
        return {height - y, x};
    case 2:
        return {width - x, height - y};
    default:
        return {y, width - x};
    }
}
} // namespace reinsirk::detail

[[nodiscard]] inline std::vector<LatticeOrientation> lattice_shape_transforms(
    std::span<const std::pair<std::int64_t, std::int64_t>> source,
    std::span<const std::pair<std::int64_t, std::int64_t>> target, bool allow_reflection = false)
{
    if (source.size() != target.size())
    {
        return {};
    }
    const auto start = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::NormalizeLatticeShape(source);
    auto goal = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::NormalizeLatticeShape(target);
    std::ranges::sort(goal.points);
    std::vector</*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::TranslatedLatticePoint> transformed(source.size());
    std::vector<LatticeOrientation> result;
    const int orientations = allow_reflection ? 8 : 4;
    for (int orientation = 0; orientation < orientations; ++orientation)
    {
        const auto [width, height] = orientation % 2 == 0 ? std::pair{start.width, start.height}
                                                         : std::pair{start.height, start.width};
        if (width != goal.width || height != goal.height)
        {
            continue;
        }
        for (std::size_t i = 0; i < start.points.size(); ++i)
        {
            transformed[i] = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ReorientLatticePoint(start.points[i], start.width, start.height, orientation);
        }
        std::ranges::sort(transformed);
        if (transformed == goal.points)
        {
            result.push_back(static_cast<LatticeOrientation>(orientation));
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
