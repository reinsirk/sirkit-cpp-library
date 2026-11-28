// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/exact_cover.md
// 参考（Algorithm X / Dancing Links）: https://arxiv.org/abs/cs/0011047
//
// API一覧
// ExactCover(required, optional=0): 必須列[0,required)はちょうど1回、任意列は高々1回覆う。
// add_row(columns): 必須列を1つ以上含む行を追加して、0始まりの行番号を返す。O(k log k)時間。
// solve(): 解を構成する行番号を1組返す。解がなければnullopt。
// for_each_solution(visit): 解ごとに行番号のspanを渡す。visitがfalseを返すと打ち切る。
// 列数をC、全行に含まれる列の延べ数をMとする。探索準備はO(C+M)、使用領域もO(C+M)。探索は最悪指数時間。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EXACT_COVER_DEFINED
#define SIRKIT_EXACT_COVER_DEFINED

namespace detail
{
class ExactCoverSearch
{
    struct Node
    {
        std::size_t left = 0;
        std::size_t right = 0;
        std::size_t up = 0;
        std::size_t down = 0;
        std::size_t column = 0;
        std::size_t row = 0;
    };

public:
    ExactCoverSearch(std::size_t required, std::size_t columns, const std::vector<std::vector<std::size_t>> &rows)
        : nodes_(columns + 1), column_sizes_(columns + 1)
    {
        nodes_[0].left = required;
        nodes_[0].right = required == 0 ? 0 : 1;
        chosen_.reserve(required);
        for (std::size_t column = 1; column <= columns; ++column)
        {
            auto &node = nodes_[column];
            node.up = node.down = column;
            if (column <= required)
            {
                node.left = column - 1;
                node.right = column == required ? 0 : column + 1;
            }
            else
            {
                node.left = node.right = column;
            }
        }
        for (std::size_t row = 0; row < rows.size(); ++row)
        {
            const auto first = nodes_.size();
            const auto last = first + rows[row].size() - 1;
            for (const auto index : rows[row])
            {
                const auto column = index + 1;
                const auto id = nodes_.size();
                const auto up = nodes_[column].up;
                nodes_.push_back({id == first ? last : id - 1, id == last ? first : id + 1, up, column, column, row});
                nodes_[up].down = nodes_[column].up = id;
                ++column_sizes_[column];
            }
        }
    }

    template <class Visitor>
    bool Run(Visitor &visit)
    {
        const auto first = nodes_[0].right;
        if (first == 0)
        {
            return visit(std::span<const std::size_t>(chosen_));
        }
        auto column = first;
        for (auto next = nodes_[first].right; next != 0; next = nodes_[next].right)
        {
            if (column_sizes_[next] < column_sizes_[column])
            {
                column = next;
            }
        }
        Cover(column);
        for (auto row = nodes_[column].down; row != column; row = nodes_[row].down)
        {
            chosen_.push_back(nodes_[row].row);
            for (auto next = nodes_[row].right; next != row; next = nodes_[next].right)
            {
                Cover(nodes_[next].column);
            }
            if (!Run(visit))
            {
                return false;
            }
            for (auto next = nodes_[row].left; next != row; next = nodes_[next].left)
            {
                Uncover(nodes_[next].column);
            }
            chosen_.pop_back();
        }
        Uncover(column);
        return true;
    }

private:
    void Cover(std::size_t column)
    {
        const auto &header = nodes_[column];
        nodes_[header.left].right = header.right;
        nodes_[header.right].left = header.left;
        for (auto row = header.down; row != column; row = nodes_[row].down)
        {
            for (auto next = nodes_[row].right; next != row; next = nodes_[next].right)
            {
                const auto &node = nodes_[next];
                nodes_[node.up].down = node.down;
                nodes_[node.down].up = node.up;
                --column_sizes_[node.column];
            }
        }
    }

    void Uncover(std::size_t column)
    {
        const auto &header = nodes_[column];
        for (auto row = header.up; row != column; row = nodes_[row].up)
        {
            for (auto next = nodes_[row].left; next != row; next = nodes_[next].left)
            {
                const auto &node = nodes_[next];
                nodes_[node.up].down = next;
                nodes_[node.down].up = next;
                ++column_sizes_[node.column];
            }
        }
        nodes_[header.left].right = column;
        nodes_[header.right].left = column;
    }

    std::vector<Node> nodes_;
    std::vector<std::size_t> column_sizes_;
    std::vector<std::size_t> chosen_;
};
} // namespace detail

class ExactCover
{
public:
    explicit ExactCover(std::size_t required, std::size_t optional = 0)
        : required_(required)
    {
        constexpr auto kLimit = std::numeric_limits<std::size_t>::max();
        if (required >= kLimit || optional > kLimit - required - 1)
        {
            throw std::length_error("ExactCover: column count overflows");
        }
        columns_ = required + optional;
    }

    std::size_t add_row(std::span<const std::size_t> columns)
    {
        std::vector<std::size_t> ordered(columns.begin(), columns.end());
        std::ranges::sort(ordered);
        if (ordered.empty() || ordered.front() >= required_ || ordered.back() >= columns_ ||
            std::ranges::adjacent_find(ordered) != ordered.end())
        {
            throw std::invalid_argument("ExactCover: invalid row columns");
        }
        rows_.push_back(std::move(ordered));
        return rows_.size() - 1;
    }

    [[nodiscard]] std::optional<std::vector<std::size_t>> solve() const
    {
        std::optional<std::vector<std::size_t>> answer;
        for_each_solution([&](std::span<const std::size_t> rows)
                          {
                              answer.emplace(rows.begin(), rows.end());
                              return false;
                          });
        return answer;
    }

    template <class Visitor>
    void for_each_solution(Visitor &&visit) const
    {
        detail::ExactCoverSearch(required_, columns_, rows_).Run(visit);
    }

private:
    std::size_t required_;
    std::size_t columns_ = 0;
    std::vector<std::vector<std::size_t>> rows_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
