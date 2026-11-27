// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Range_Xor_Basis.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_xor_basis.md
//
// API一覧
// RangeXorQuery {left, right, x}: 0始まりの半開区間[left,right)と初期値x。
// RangeXorResult {can_make, maximum, rank}: xを作れるか、xと部分集合XORの排他的論理和の最大値、基底の次元。
// range_xor_queries(values,queries): uint64_t列の各区間について、問い合わせ順に答えを返す。空集合のXORは0。
// n要素、q問、w=64ビットとしてO((n+q)w+q log(q+1))時間、O(w+q)領域。入力不変。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_XOR_BASIS_DEFINED
#define SIRKIT_RANGE_XOR_BASIS_DEFINED
struct RangeXorQuery
{
    std::size_t left;
    std::size_t right;
    std::uint64_t x = 0;
};

struct RangeXorResult
{
    bool can_make;
    std::uint64_t maximum;
    unsigned rank;
};

inline std::vector<RangeXorResult> range_xor_queries(std::span<const std::uint64_t> values,
                                                     std::span<const RangeXorQuery> queries)
{
    std::vector<std::size_t> order(queries.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b)
              {
                  return queries[a].right < queries[b].right;
              });
    struct Pivot
    {
        std::uint64_t value = 0;
        // 1始まりにして0を未設定と区別する。
        std::size_t position = 0;
    };
    constexpr int kBits = std::numeric_limits<std::uint64_t>::digits;
    std::array<Pivot, kBits> pivots{};
    std::vector<RangeXorResult> result(queries.size());
    std::size_t processed = 0;
    for (const auto id : order)
    {
        const auto [left, right, x] = queries[id];
        while (processed < right)
        {
            Pivot current{values[processed], processed + 1};
            // 同じ先頭ビットでは、より右に出現したベクトルを残す。
            while (current.value != 0)
            {
                const auto bit = std::bit_width(current.value) - 1;
                if (pivots[bit].position < current.position)
                {
                    std::swap(pivots[bit], current);
                }
                current.value ^= pivots[bit].value;
            }
            ++processed;
        }
        std::uint64_t remainder = x, maximum = x;
        unsigned rank = 0;
        for (int b = kBits - 1; b >= 0; --b)
        {
            const auto &pivot = pivots[b];
            if (pivot.position <= left)
            {
                continue;
            }
            ++rank;
            if ((remainder >> b) & 1)
            {
                remainder ^= pivot.value;
            }
            maximum = std::max(maximum, maximum ^ pivot.value);
        }
        result[id] = {remainder == 0, maximum, rank};
    }
    return result;
}
#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
