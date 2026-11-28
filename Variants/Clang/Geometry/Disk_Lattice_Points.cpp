// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Geometry/Disk_Lattice_Points.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/disk_lattice_points.md
// 参考（Verify問題）: https://atcoder.jp/contests/abc191/tasks/abc191_d
//
// API一覧
// geo::count_disk_lattice_points(x,y,radius,unit=1): 中心(x/unit,y/unit)、半径radius/unitの円の整数格子点を数える。
// {closed,boundary}は円周を含む総数と円周だけの個数。負の中心座標も可。
// unit>0、0<=radius<=2^31-1。範囲外はinvalid_argument。O(1+radius/unit)時間・O(1)領域。
// 中心と半径は同じ倍率unitで整数化して渡す。浮動小数点演算は使わない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <stdexcept>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DISK_LATTICE_POINTS_DEFINED
#define SIRKIT_DISK_LATTICE_POINTS_DEFINED

namespace geo
{
struct DiskLatticePointCount
{
    std::uint64_t closed = 0;
    std::uint64_t boundary = 0;
};

[[nodiscard]] inline DiskLatticePointCount count_disk_lattice_points(std::int64_t x, std::int64_t y, std::int64_t radius,
                                                                     std::int64_t unit = 1)
{
    if (unit <= 0 || radius < 0 || radius > std::numeric_limits<std::int32_t>::max())
    {
        throw std::invalid_argument("count_disk_lattice_points: invalid unit or radius");
    }
    const auto reduced_center = [&](std::int64_t coordinate)
    {
        const auto residue = std::abs(coordinate % unit);
        return std::min(residue, unit - residue);
    };
    x = reduced_center(x);
    y = reduced_center(y);
    if (x > radius || y > radius)
    {
        return {};
    }

    const auto ceil_units = [&](std::int64_t coordinate)
    {
        return coordinate / unit + (coordinate % unit > 0);
    };
    const auto floor_units = [&](std::int64_t coordinate)
    {
        return coordinate / unit - (coordinate % unit < 0);
    };
    const auto low_y = ceil_units(y - radius);
    const auto high_y = floor_units(y + radius);
    const auto radius_squared = radius * radius;
    const auto squared_vertical_distance = [&](std::int64_t row)
    {
        const auto dy = row * unit - y;
        return dy * dy;
    };
    DiskLatticePointCount result;
    const auto scan = [&](std::int64_t column, std::int64_t stop, std::int64_t step)
    {
        auto lower = low_y;
        auto upper = high_y;
        for (; column != stop; column += step)
        {
            const auto dx = column * unit - x;
            const auto remaining = radius_squared - dx * dx;
            while (lower <= upper && squared_vertical_distance(upper) > remaining)
            {
                --upper;
            }
            while (lower <= upper && squared_vertical_distance(lower) > remaining)
            {
                ++lower;
            }
            if (lower > upper)
            {
                break;
            }
            result.closed += static_cast<std::uint64_t>(upper - lower + 1);
            result.boundary += static_cast<std::uint64_t>(squared_vertical_distance(lower) == remaining);
            if (lower != upper)
            {
                result.boundary += static_cast<std::uint64_t>(squared_vertical_distance(upper) == remaining);
            }
        }
    };
    // 中心から遠ざかる順に列を走査すると、有効な行の区間は狭くなるだけ。
    scan(0, floor_units(x + radius) + 1, 1);
    scan(-1, ceil_units(x - radius) - 1, -1);
    return result;
}
} // namespace geo
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
