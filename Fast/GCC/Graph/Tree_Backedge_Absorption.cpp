// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Tree_Backedge_Absorption.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_backedge_absorption.md
//
// API一覧
// TreeBackedgeAbsorption(parents, backward_edges): 親から子へ進む根付き木に、葉以外から真の祖先への辺を加える。
//   parents[v]=-1 は唯一の根。頂点は0始まり。重複辺は選択肢に別々に数える。構築 O(n+m)。
// expected_steps<Field>(): 根から各出辺を等確率で選び、葉に着くまでの移動回数の期待値。
// expected_cost<Field>(costs): 頂点vを出発する度に払う costs[v] の合計の期待値。
//   Field は0,1と辺本数を表せる体。消去中の係数が0なら std::nullopt を返す。
//   n頂点、追加m辺で O((n+m) log²(n+m+1)) 回の体演算、うち除算は O(n)。領域 O(n+m)。
// 部分木内の頂点を消去し、祖先に戻るときの係数と部分木滞在中の期待コストをまとめる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include <optional>
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

#ifndef SIRKIT_TREE_BACKEDGE_ABSORPTION_DEFINED
#define SIRKIT_TREE_BACKEDGE_ABSORPTION_DEFINED

class TreeBackedgeAbsorption
{
public:
    explicit TreeBackedgeAbsorption(
        std::span<const int> parents,
        std::span<const std::pair<int, int>> backward_edges = {})
        : layout_(parents), children_(parents.size()), backward_edges_(parents.size())
    {
        if (std::ranges::count(parents, -1) != 1)
        {
            throw std::invalid_argument("TreeBackedgeAbsorption: expected one root");
        }
        for (const int vertex : layout_.order())
        {
            const int parent = parents[vertex];
            if (parent != -1)
            {
                children_[parent].push_back(vertex);
            }
        }
        for (const auto &[from, to] : backward_edges)
        {
            if (from == to || !layout_.contains(to, from) || children_[from].empty())
            {
                throw std::invalid_argument("TreeBackedgeAbsorption: invalid backward edge");
            }
            backward_edges_[from].push_back(to);
        }
    }

    template <class Field>
    [[nodiscard]] std::optional<Field> expected_steps() const
    {
        return Solve<Field>([](int)
                            {
                                return Field{1};
                            });
    }

    template <class Field>
    [[nodiscard]] std::optional<Field> expected_cost(std::span<const Field> costs) const
    {
        if (costs.size() != static_cast<std::size_t>(layout_.size()))
        {
            throw std::invalid_argument("TreeBackedgeAbsorption: wrong number of costs");
        }
        return Solve<Field>([costs](int vertex)
                            {
                                return costs[vertex];
                            });
    }

private:
    SubtreeIntervals layout_;
    std::vector<std::vector<int>> children_;
    std::vector<std::vector<int>> backward_edges_;

    template <class Field>
    struct Row
    {
        Field cost{};
        Field scale{1}, inverse_scale{1};
        std::map<int, Field> coefficients;
    };

    template <class Field, class Reward>
    std::optional<Field> Solve(Reward reward) const
    {
        auto order = layout_.order();
        if (order.empty())
        {
            return Field{};
        }
        std::vector<Row<Field>> rows(order.size());
        for (auto it = order.rbegin(); it != order.rend(); ++it)
        {
            const int vertex = *it;
            const auto &children = children_[vertex];
            if (children.empty())
            {
                continue;
            }
            const auto &backward = backward_edges_[vertex];
            const Field degree(children.size() + backward.size());
            Field value = degree * reward(vertex);
            int largest = children.front();
            for (const int child : children)
            {
                value += rows[child].cost;
                if (rows[child].coefficients.size() > rows[largest].coefficients.size())
                {
                    largest = child;
                }
            }
            auto &row = rows[vertex];
            std::swap(row, rows[largest]);
            for (const int child : children)
            {
                auto &part = rows[child];
                const Field conversion = part.scale * row.inverse_scale;
                for (const auto &[ancestor, coefficient] : part.coefficients)
                {
                    row.coefficients[ancestor] += conversion * coefficient;
                }
                part.coefficients.clear();
            }
            for (const int ancestor : backward)
            {
                row.coefficients[ancestor] += row.inverse_scale;
            }
            Field pivot = degree;
            const auto self = row.coefficients.find(vertex);
            if (self != row.coefficients.end())
            {
                pivot = pivot - row.scale * self->second;
                row.coefficients.erase(self);
            }
            if (pivot == Field{})
            {
                return std::nullopt;
            }
            const Field reciprocal = pivot == Field{1} ? Field{1} : Field{1} / pivot;
            row.cost = value * reciprocal;
            row.scale = row.scale * reciprocal;
            row.inverse_scale = row.inverse_scale * pivot;
        }
        return rows[order.front()].cost;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
