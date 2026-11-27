// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Xor_Basis_Intersection.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/xor_basis_intersection.md
//
// API一覧
// xor_basis_intersection(left, right): 64bit整数をGF(2)のベクトルとして、両方のXOR生成空間に共通する部分の基底を返す。
// 入力は生成元の列。0・重複・従属を許す。空間が{0}なら空の基底を返す。
// ビット幅W=64、入力の総数Nに対してO((N+1)W)時間、O(W)補助領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_XOR_BASIS_INTERSECTION_DEFINED
#define SIRKIT_XOR_BASIS_INTERSECTION_DEFINED

inline std::vector<std::uint64_t> xor_basis_intersection(std::span<const std::uint64_t> left,
                                                         std::span<const std::uint64_t> right)
{
    if (left.empty() || right.empty())
    {
        return {};
    }
    constexpr auto kWidth = std::numeric_limits<std::uint64_t>::digits;
    struct Row
    {
        std::uint64_t value{};
        std::uint64_t from_left{};
    };
    std::array<Row, kWidth> rows{};
    for (auto value : left)
    {
        while (value != 0)
        {
            auto &row = rows[std::bit_width(value) - 1];
            if (row.value == 0)
            {
                row = {value, value};
                break;
            }
            value ^= row.value;
        }
    }

    std::array<std::uint64_t, kWidth> common{};
    std::size_t rank = 0;
    for (auto value : right)
    {
        std::uint64_t from_left = 0;
        // 各行について、row.value XOR row.from_left は右の生成空間に属する。
        while (value != 0)
        {
            auto &row = rows[std::bit_width(value) - 1];
            if (row.value == 0)
            {
                row = {value, from_left};
                break;
            }
            value ^= row.value;
            from_left ^= row.from_left;
        }
        if (value != 0)
        {
            continue;
        }
        // 消去で0になったときのfrom_leftは両空間に属する。重複を除いて独立にする。
        while (from_left != 0)
        {
            auto &pivot = common[std::bit_width(from_left) - 1];
            if (pivot == 0)
            {
                pivot = from_left;
                ++rank;
                break;
            }
            from_left ^= pivot;
        }
    }
    std::vector<std::uint64_t> result;
    result.reserve(rank);
    for (const auto value : common)
    {
        if (value != 0)
        {
            result.push_back(value);
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
