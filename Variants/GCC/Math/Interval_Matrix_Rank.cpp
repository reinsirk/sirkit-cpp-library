// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Interval_Matrix_Rank.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/interval_matrix_rank.md
// 参考（区間の階数）: https://atcoder.jp/contests/abc399/editorial/12546
//
// API一覧
// interval_matrix_rank_ends<Mod=998244353>(rows,rank):
// 各左端lについて行区間[l,r)の階数がrank以上になる最小のrを返す。なければnullopt。
// rowsは長方形のuint32_t行列。各値を素数Modで割った余りとして扱う。
// rank=0なら各lの答えはl。空の行列に対する答えは空。行幅が違えばinvalid_argument。
// 行数H、列数Wに対しO(HW(W+log Mod)+H)時間、O(W^2+H)領域。


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
#ifndef SIRKIT_INTERVAL_MATRIX_RANK_DEFINED
#define SIRKIT_INTERVAL_MATRIX_RANK_DEFINED

namespace detail
{
consteval bool IntervalMatrixRankPrime(std::uint32_t mod)
{
    if (mod < 2)
    {
        return false;
    }
    for (std::uint32_t factor = 2; factor <= mod / factor; ++factor)
    {
        if (mod % factor == 0)
        {
            return false;
        }
    }
    return true;
}

template <std::uint32_t Mod>
std::uint32_t IntervalMatrixRankInverse(std::uint32_t value)
{
    std::uint32_t result = 1;
    for (std::uint32_t exponent = Mod - 2; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1u) != 0)
        {
            result = static_cast<std::uint32_t>(std::uint64_t{result} * value % Mod);
        }
        value = static_cast<std::uint32_t>(std::uint64_t{value} * value % Mod);
    }
    return result;
}

template <std::uint32_t Mod>
void NormalizeIntervalMatrixRankPivot(std::vector<std::uint32_t> &row, std::size_t column)
{
    const auto inverse = IntervalMatrixRankInverse<Mod>(row[column]);
    row[column] = 1;
    for (std::size_t i = column + 1; i < row.size(); ++i)
    {
        row[i] = static_cast<std::uint32_t>(std::uint64_t{row[i]} * inverse % Mod);
    }
}

struct TaggedIntervalMatrixPivot
{
    std::vector<std::uint32_t> row;
    std::size_t index{};
};
} // namespace detail

template <std::uint32_t Mod = 998244353>
[[nodiscard]] std::vector<std::optional<std::size_t>> interval_matrix_rank_ends(
    std::span<const std::vector<std::uint32_t>> rows, std::size_t rank)
{
    static_assert(detail::IntervalMatrixRankPrime(Mod), "interval_matrix_rank_ends: Mod must be prime");
    if (rows.empty())
    {
        return {};
    }
    const auto width = rows.front().size();
    for (const auto &row : rows)
    {
        if (row.size() != width)
        {
            throw std::invalid_argument("interval_matrix_rank_ends: inconsistent row widths");
        }
    }
    std::vector<std::optional<std::size_t>> ends(rows.size());
    if (rank == 0)
    {
        for (std::size_t left = 0; left < rows.size(); ++left)
        {
            ends[left] = left;
        }
        return ends;
    }
    if (rank > width)
    {
        return ends;
    }

    std::vector<detail::TaggedIntervalMatrixPivot> basis(width);
    std::vector<std::size_t> indexes;
    indexes.reserve(std::min(rows.size(), width));
    for (std::size_t next = rows.size(); next != 0; --next)
    {
        const auto left = next - 1;
        auto index = left;
        auto row = rows[left];
        for (auto &value : row)
        {
            value %= Mod;
        }
        for (std::size_t column = 0; column < width; ++column)
        {
            if (row[column] == 0)
            {
                continue;
            }
            auto &pivot = basis[column];
            if (pivot.row.empty())
            {
                detail::NormalizeIntervalMatrixRankPivot<Mod>(row, column);
                pivot = {std::move(row), index};
                break;
            }
            // 同じ列のピボットは右端が小さい方を残す。
            if (index < pivot.index)
            {
                detail::NormalizeIntervalMatrixRankPivot<Mod>(row, column);
                row.swap(pivot.row);
                std::swap(index, pivot.index);
            }
            const auto factor = row[column];
            row[column] = 0;
            for (std::size_t i = column + 1; i < width; ++i)
            {
                const auto sub = static_cast<std::uint32_t>(std::uint64_t{factor} * pivot.row[i] % Mod);
                row[i] = row[i] >= sub ? row[i] - sub : row[i] + (Mod - sub);
            }
        }
        indexes.clear();
        for (const auto &pivot : basis)
        {
            if (!pivot.row.empty())
            {
                indexes.push_back(pivot.index);
            }
        }
        if (indexes.size() >= rank)
        {
            std::sort(indexes.begin(), indexes.end());
            ends[left] = indexes[rank - 1] + 1;
        }
    }
    return ends;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
