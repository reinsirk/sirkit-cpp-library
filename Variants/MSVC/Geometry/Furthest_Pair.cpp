// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Geometry/Furthest_Pair.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/furthest_pair.md
//
// API一覧
// furthest_pair(points): int32_t座標の最遠点対の距離二乗と元の添字を返す。2点未満ならnullopt。
// FurthestPairResult{distance_squared,first_index,second_index}: 距離二乗はuint64_t、添字は0始まりで昇順。
// 同じ座標の点も扱う。同距離の対は任意。距離二乗がuint64_tを超える場合はoverflow_error。
// 入力を変更せず、N点でO(N log(N+1))時間・O(N)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FURTHEST_PAIR_DEFINED
#define SIRKIT_FURTHEST_PAIR_DEFINED

struct FurthestPairResult
{
    std::uint64_t distance_squared;
    std::size_t first_index;
    std::size_t second_index;
};

namespace detail
{
struct FurthestPairPoint
{
    std::pair<std::int32_t, std::int32_t> coordinates;
    std::size_t index;
};

inline std::uint64_t FurthestPairMagnitude(std::int64_t value)
{
    // 呼出側はint32_tの座標差だけを渡す。
    return static_cast<std::uint64_t>(value < 0 ? -value : value);
}

inline int FurthestPairCrossSign(const FurthestPairPoint &a, const FurthestPairPoint &b,
                                 const FurthestPairPoint &c, const FurthestPairPoint &d)
{
    const auto ab_x = static_cast<std::int64_t>(b.coordinates.first) - a.coordinates.first;
    const auto ab_y = static_cast<std::int64_t>(b.coordinates.second) - a.coordinates.second;
    const auto cd_x = static_cast<std::int64_t>(d.coordinates.first) - c.coordinates.first;
    const auto cd_y = static_cast<std::int64_t>(d.coordinates.second) - c.coordinates.second;

    // 各積の大きさはuint64_tに収まる。差は求めず、積の順序だけ比較する。
    const auto left = FurthestPairMagnitude(ab_x) * FurthestPairMagnitude(cd_y);
    const auto right = FurthestPairMagnitude(ab_y) * FurthestPairMagnitude(cd_x);
    const bool left_negative = left != 0 && ((ab_x < 0) != (cd_y < 0));
    const bool right_negative = right != 0 && ((ab_y < 0) != (cd_x < 0));
    if (left_negative != right_negative)
    {
        return left_negative ? -1 : 1;
    }
    const int order = (left > right) - (left < right);
    return left_negative ? -order : order;
}

inline void ImproveFurthestPair(FurthestPairResult &best, const FurthestPairPoint &a, const FurthestPairPoint &b)
{
    const auto dx = FurthestPairMagnitude(static_cast<std::int64_t>(a.coordinates.first) - b.coordinates.first);
    const auto dy = FurthestPairMagnitude(static_cast<std::int64_t>(a.coordinates.second) - b.coordinates.second);
    const auto square_x = dx * dx;
    const auto squared = square_x + dy * dy;
    if (squared < square_x)
    {
        throw std::overflow_error("furthest_pair: squared distance exceeds uint64_t");
    }
    if (squared > best.distance_squared)
    {
        best = {squared, std::min(a.index, b.index), std::max(a.index, b.index)};
    }
}

inline std::vector<FurthestPairPoint> FurthestPairHull(std::vector<FurthestPairPoint> &points)
{
    std::ranges::sort(points, {}, &FurthestPairPoint::coordinates);
    const auto duplicate = std::ranges::unique(points, {}, &FurthestPairPoint::coordinates);
    points.erase(duplicate.begin(), duplicate.end());
    if (points.size() <= 2)
    {
        return std::move(points);
    }

    std::vector<FurthestPairPoint> hull;
    hull.reserve(points.size());
    const auto append = [&](const FurthestPairPoint &point, std::size_t first)
    {
        while (hull.size() - first >= 2 &&
               FurthestPairCrossSign(hull[hull.size() - 2], hull.back(), hull.back(), point) <= 0)
        {
            hull.pop_back();
        }
        hull.push_back(point);
    };
    for (const auto &point : points)
    {
        append(point, 0);
    }
    const auto first = hull.size() - 1;
    for (auto i = points.size() - 1; i != 0; --i)
    {
        append(points[i - 1], first);
    }
    hull.pop_back();
    return hull;
}
} // namespace detail

inline std::optional<FurthestPairResult> furthest_pair(
    std::span<const std::pair<std::int32_t, std::int32_t>> points)
{
    if (points.size() < 2)
    {
        return std::nullopt;
    }
    FurthestPairResult best{0, 0, 1};
    if (points.size() == 2)
    {
        detail::ImproveFurthestPair(best, {points[0], 0}, {points[1], 1});
        return best;
    }

    std::vector<detail::FurthestPairPoint> indexed;
    indexed.reserve(points.size());
    for (std::size_t i = 0; i < points.size(); ++i)
    {
        indexed.push_back({points[i], i});
    }
    const auto hull = detail::FurthestPairHull(indexed);
    if (hull.size() == 1)
    {
        return best;
    }
    if (hull.size() == 2)
    {
        detail::ImproveFurthestPair(best, hull[0], hull[1]);
        return best;
    }

    const auto next = [&](std::size_t i)
    {
        return i + 1 == hull.size() ? 0 : i + 1;
    };
    std::size_t opposite = 1;
    for (std::size_t i = 0; i < hull.size(); ++i)
    {
        const auto j = next(i);
        auto after = next(opposite);
        int turn = detail::FurthestPairCrossSign(hull[i], hull[j], hull[opposite], hull[after]);
        while (turn > 0)
        {
            opposite = after;
            after = next(opposite);
            turn = detail::FurthestPairCrossSign(hull[i], hull[j], hull[opposite], hull[after]);
        }
        detail::ImproveFurthestPair(best, hull[i], hull[opposite]);
        detail::ImproveFurthestPair(best, hull[j], hull[opposite]);
        if (turn == 0)
        {
            // 対向辺が平行な場合は、その両端を候補に含める。
            detail::ImproveFurthestPair(best, hull[i], hull[after]);
            detail::ImproveFurthestPair(best, hull[j], hull[after]);
        }
    }
    return best;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
