// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/DSU_On_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dsu_on_tree.md
//
// API一覧
// DsuOnTree(parents): 親配列の森を前処理。頂点は0始まり。根の親は-1。O(n)時間・領域。
// DsuOnTree(intervals): 構築済みのSubtreeIntervalsを共有して前処理。O(n)時間・追加領域。
// size(): 頂点数。O(1)。
// run(add,remove,answer): answer(v)でvの部分木全体が登録されるようにadd(u), remove(u)を呼ぶ。
//   呼び出し前後の集合は空。頂点ごとにanswerを1回呼ぶ。走査順は規定しない。
//   add/removeは逆操作。コールバックがO(1)なら O(n log(n+1)) 時間、O(1) 追加領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <functional>
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

#ifndef SIRKIT_DSU_ON_TREE_DEFINED
#define SIRKIT_DSU_ON_TREE_DEFINED

class DsuOnTree
{
public:
    explicit DsuOnTree(std::span<const int> parents)
        : DsuOnTree(SubtreeIntervals(parents))
    {
    }

    explicit DsuOnTree(SubtreeIntervals intervals)
        : intervals_(std::move(intervals)),
          heavy_(intervals_.size(), -1),
          keep_(intervals_.size())
    {
        const auto vertices = intervals_.order();
        schedule_.reserve(vertices.size());
        std::vector<int> pending;
        for (int i = 0; i < size();)
        {
            const int root = vertices[i];
            pending.push_back(root);
            i = intervals_.range(root).second;
        }
        while (!pending.empty())
        {
            const int v = pending.back();
            pending.pop_back();
            schedule_.push_back(v);
            const auto [first, last] = intervals_.range(v);
            int largest = 0;
            for (int i = first + 1; i < last;)
            {
                const int child = vertices[i];
                const int end = intervals_.range(child).second;
                if (end - i > largest)
                {
                    largest = end - i;
                    heavy_[v] = child;
                }
                i = end;
            }
            for (int i = first + 1; i < last;)
            {
                const int child = vertices[i];
                if (child != heavy_[v])
                {
                    pending.push_back(child);
                }
                i = intervals_.range(child).second;
            }
            if (const int child = heavy_[v]; child != -1)
            {
                keep_[child] = true;
                pending.push_back(child);
            }
        }
        std::ranges::reverse(schedule_);
    }

    [[nodiscard]] int size() const noexcept
    {
        return intervals_.size();
    }

    template <typename Add, typename Remove, typename Answer>
    void run(Add &&add, Remove &&remove, Answer &&answer) const
    {
        const auto vertices = intervals_.order();
        for (int vertex : schedule_)
        {
            const auto [first, last] = intervals_.range(vertex);
            if (const int heavy = heavy_[vertex]; heavy != -1)
            {
                const auto [begin, end] = intervals_.range(heavy);
                for (int i = first; i < begin; ++i)
                {
                    std::invoke(add, vertices[i]);
                }
                for (int i = end; i < last; ++i)
                {
                    std::invoke(add, vertices[i]);
                }
            }
            else
            {
                std::invoke(add, vertex);
            }
            std::invoke(answer, vertex);
            if (!keep_[vertex])
            {
                for (int i = first; i < last; ++i)
                {
                    std::invoke(remove, vertices[i]);
                }
            }
        }
    }

private:
    SubtreeIntervals intervals_;
    std::vector<int> heavy_;
    std::vector<bool> keep_;
    std::vector<int> schedule_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
