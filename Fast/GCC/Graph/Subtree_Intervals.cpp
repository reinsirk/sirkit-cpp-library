// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Subtree_Intervals.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subtree_intervals.md
//
// API一覧
// SubtreeIntervals(parents): 親配列の森をDFS行きがけ順に並べる。parents[v]=-1は根。O(n)時間・領域。
// range(v): vの部分木の半開区間{left,right}。index(v): vの位置。vertex_at(i): 位置iの頂点。
// order(): 行きがけ順の頂点番号を非所有spanで返す。contains(u,v): vがuの部分木に含まれるか。
// size(): 頂点数。構築以外はO(1)。頂点と位置は0始まり、n<=INT_MAX。根と子は頂点番号の小さい順に訪問。
// 根のない閉路と範囲外の親は例外。コピーは前処理を共有しO(1)。移動元は空。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBTREE_INTERVALS_DEFINED
#define SIRKIT_SUBTREE_INTERVALS_DEFINED

namespace detail
{
inline int checked_subtree_interval_size(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("SubtreeIntervals: too many vertices");
    }
    return static_cast<int>(size);
}
}

class SubtreeIntervals
{
public:
    explicit SubtreeIntervals(std::span<const int> parents)
        : data_(std::make_shared<const Data>(parents))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(order().size());
    }

    [[nodiscard]] std::span<const int> order() const noexcept
    {
        return data_ ? std::span<const int>(data_->order) : std::span<const int>{};
    }

    [[nodiscard]] int index(int vertex) const
    {
        CheckIndex(vertex);
        return data_->first[vertex];
    }

    [[nodiscard]] std::pair<int, int> range(int vertex) const
    {
        const auto start = index(vertex);
        return {start, start + data_->count[vertex]};
    }

    [[nodiscard]] bool contains(int ancestor, int vertex) const
    {
        const auto [left, right] = range(ancestor);
        const auto at = index(vertex);
        return left <= at && at < right;
    }

    [[nodiscard]] int vertex_at(int position) const
    {
        CheckIndex(position);
        return data_->order[position];
    }

private:
    struct Data
    {
        std::vector<int> order;
        std::vector<int> first;
        std::vector<int> count;

        explicit Data(std::span<const int> parents)
        {
            const auto n = detail::checked_subtree_interval_size(parents.size());
            std::vector<int> child(n, -1), next(n, -1), work;
            work.reserve(n);
            for (int v = 0; v < n; ++v)
            {
                const int p = parents[v];
                if (p == -1)
                {
                    work.push_back(v);
                }
                else
                {
                    if (p < 0 || p >= n)
                    {
                        throw std::out_of_range("SubtreeIntervals: parent");
                    }
                    next[v] = child[p];
                    child[p] = v;
                }
            }

            first.resize(n);
            order.reserve(n);
            std::reverse(work.begin(), work.end());
            while (!work.empty())
            {
                const int v = work.back();
                work.pop_back();
                first[v] = static_cast<int>(order.size());
                order.push_back(v);
                for (int w = child[v]; w != -1; w = next[w])
                {
                    work.push_back(w);
                }
            }
            if (order.size() != parents.size())
            {
                throw std::invalid_argument("SubtreeIntervals: cycle");
            }

            count.assign(n, 1);
            for (auto current = order.rbegin(); current != order.rend(); ++current)
            {
                const int parent = parents[*current];
                if (parent != -1)
                {
                    count[parent] += count[*current];
                }
            }
        }
    };

    std::shared_ptr<const Data> data_;

    void CheckIndex([[maybe_unused]] int index) const
    {
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
