// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Assignment.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/assignment.md
//
// API一覧
// minimum_assignment(cost): n×m (n≤m) の全行に異なる列を割り当て、合計費用を最小化する。
// AssignmentResult{cost,column}: 最小費用と各行の割当列。空行列なら費用0・空列。
// |cost[i][j]|≤INT64_MAX/(2*(n+1)+1)。O(n²m)時間・O(n+m)追加領域。

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
#ifndef SIRKIT_ASSIGNMENT_DEFINED
#define SIRKIT_ASSIGNMENT_DEFINED

struct AssignmentResult
{
    std::int64_t cost;
    std::vector<std::size_t> column;
};

inline AssignmentResult minimum_assignment(std::span<const std::vector<std::int64_t>> cost)
{
    const auto rows = cost.size();
    if (rows == 0)
    {
        return {0, {}};
    }
    const auto cols = cost.front().size();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    if (rows > cols)
    {
        throw std::length_error("minimum_assignment: unsupported dimensions");
    }
    const auto limit = kMax / (2 * static_cast<std::int64_t>(rows) + 3);
    std::int64_t least = 0;
    for (const auto &row : cost)
    {
        for (auto value : row)
        {
            if (value < -limit || value > limit)
            {
                throw std::overflow_error("minimum_assignment: cost outside supported range");
            }
            least = std::min(least, value);
        }
    }

    std::vector<std::int64_t> row_price(rows + 1), column_price(cols + 1), distance(cols + 1);
    std::vector<std::size_t> matched_row(cols + 1), previous(cols + 1);
    std::vector<unsigned char> used(cols + 1);
    for (std::size_t row = 1; row <= rows; ++row)
    {
        matched_row[0] = row;
        std::fill(distance.begin() + 1, distance.end(), kMax);
        std::fill(used.begin(), used.end(), 0);
        std::size_t column = 0;
        // 全費用を非負へ平行移動し、増加する双対変数の総量を抑える。
        do
        {
            used[column] = 1;
            const auto active_row = matched_row[column];
            std::size_t next_column = 0;
            auto delta = kMax;
            for (std::size_t candidate = 1; candidate <= cols; ++candidate)
            {
                if (used[candidate])
                {
                    continue;
                }
                const auto reduced = (cost[active_row - 1][candidate - 1] - least) -
                                     row_price[active_row] - column_price[candidate];
                if (reduced < distance[candidate])
                {
                    previous[candidate] = column;
                    distance[candidate] = reduced;
                }
                if (distance[candidate] < delta)
                {
                    delta = distance[candidate];
                    next_column = candidate;
                }
            }
            for (std::size_t k = 0; k <= cols; ++k)
            {
                if (used[k])
                {
                    row_price[matched_row[k]] += delta;
                    column_price[k] -= delta;
                }
                else
                {
                    distance[k] -= delta;
                }
            }
            column = next_column;
        } while (matched_row[column] != 0);
        do
        {
            const auto predecessor = previous[column];
            matched_row[column] = matched_row[predecessor];
            column = predecessor;
        } while (column != 0);
    }

    AssignmentResult result{0, std::vector<std::size_t>(rows)};
    for (std::size_t column = 1; column <= cols; ++column)
    {
        const auto row = matched_row[column];
        if (row != 0)
        {
            result.column[row - 1] = column - 1;
            result.cost += cost[row - 1][column - 1];
        }
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
