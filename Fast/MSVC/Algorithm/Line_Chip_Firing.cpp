// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Line_Chip_Firing.cpp
// 参考（整数直線上のchip firing）: https://www.dam.brown.edu/people/cklivans/Confluence.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/line_chip_firing.md
//
// API一覧
// line_chip_firing(positions): 同じ位置xの2個をx-1とx+1へ移し、重複がなくなるまで続ける。
// LineChipFiringResult{positions, operations}: 最終位置の昇順列と操作の総回数。操作順によらない。
// 入力はint64_tの位置列。個数n>2^20ならlength_error、最終位置がint64_tを超えるとoverflow_error。
// O(1+n log(n+1))時間、O(1+n)領域。浮動小数点や拡張整数型は不要。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LINE_CHIP_FIRING
#define SIRKIT_LINE_CHIP_FIRING

struct LineChipFiringResult
{
    std::vector<std::int64_t> positions;
    std::uint64_t operations = 0;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
struct LineChipCluster
{
    std::int64_t first, count, hole_offset;

    [[nodiscard]] constexpr std::int64_t width() const
    {
        return count + (hole_offset >= 0);
    }

    [[nodiscard]] constexpr std::int64_t last() const
    {
        return first + (width() - 1);
    }

    [[nodiscard]] constexpr std::int64_t local_sum(std::int64_t offset) const
    {
        const auto length = width();
        const auto full = offset * length + length * (length - 1) / 2;
        return hole_offset < 0 ? full : full - (offset + hole_offset);
    }

    [[nodiscard]] constexpr std::int64_t local_square_sum(std::int64_t offset) const
    {
        const auto length = width();
        const auto last_index = length - 1;
        const auto full = length * offset * offset + offset * length * last_index +
                          length * last_index * (2 * length - 1) / 6;
        const auto missing = offset + hole_offset;
        return hole_offset < 0 ? full : full - missing * missing;
    }
};

[[nodiscard]] inline std::int64_t checked_line_chip_coordinate(std::int64_t origin, std::int64_t offset)
{
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    if ((offset < 0 && origin < kMin - offset) || (offset > 0 && origin > kMax - offset))
    {
        throw std::overflow_error("line_chip_firing: final coordinate outside int64_t");
    }
    return origin + offset;
}

[[nodiscard]] inline LineChipCluster merge_line_chip_clusters(const LineChipCluster &a, const LineChipCluster &b,
                                                              std::uint64_t &operations)
{
    // 重なる2区間の相対座標だけを使う。原点の値にかかわらず、距離は駒の個数以下。
    const auto origin = std::min(a.first, b.first);
    const auto a_offset = a.first - origin;
    const auto b_offset = b.first - origin;
    const auto count = a.count + b.count;
    const auto sum = a.local_sum(a_offset) + b.local_sum(b_offset);
    const auto excess = sum - count * (count - 1) / 2;
    auto left = excess / count;
    auto remainder = excess % count;
    if (remainder < 0)
    {
        --left;
        remainder += count;
    }
    // 連続区間の座標和の差を、右端を1つ伸ばして内部の1点を抜くことで合わせる。
    const auto hole = remainder == 0 ? -1 : count - remainder;
    const LineChipCluster merged{checked_line_chip_coordinate(origin, left), count, hole};
    static_cast<void>(checked_line_chip_coordinate(origin, left + merged.width() - 1));
    // 1回の操作で座標の二乗和は2増える。同じ原点で各群を比較する。
    const auto delta = merged.local_square_sum(left) - a.local_square_sum(a_offset) - b.local_square_sum(b_offset);
    operations += static_cast<std::uint64_t>(delta / 2);
    return merged;
}
} // namespace reinsirk::detail

[[nodiscard]] inline LineChipFiringResult line_chip_firing(std::span<const std::int64_t> positions)
{
    if (positions.size() > (std::size_t{1} << 20))
    {
        throw std::length_error("line_chip_firing: too many chips");
    }
    std::vector<std::int64_t> sorted(positions.begin(), positions.end());
    std::ranges::sort(sorted);
    std::vector<detail::LineChipCluster> groups;
    groups.reserve(sorted.size());
    LineChipFiringResult result;
    for (const auto position : sorted)
    {
        detail::LineChipCluster current{position, 1, -1};
        while (!groups.empty() && groups.back().last() >= current.first)
        {
            current = detail::merge_line_chip_clusters(groups.back(), current, result.operations);
            groups.pop_back();
        }
        groups.push_back(current);
    }
    result.positions.reserve(sorted.size());
    for (const auto &group : groups)
    {
        for (std::int64_t i = 0; i < group.width(); ++i)
        {
            if (i != group.hole_offset)
            {
                result.positions.push_back(group.first + i);
            }
        }
    }
    return result;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
