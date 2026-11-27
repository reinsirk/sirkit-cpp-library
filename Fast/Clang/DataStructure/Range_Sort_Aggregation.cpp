// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Range_Sort_Aggregation.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_sort_aggregation.md
//
// API一覧
// RangeSortAggregation<S,op,e>(items): {int32_tキー,S値}の列。O(n+1)時間・領域。
// sort(l,r,descending=false): [l,r)をキーで昇順／降順に整列。
// set(i,key,value): 位置iのキーと値を置換。get(i): そのキーと値。各O(W+log(n+1))。
// prod(l,r): 現在の順序で非可換の区間積を返す。空区間はe()。O(W+log(n+1))。
// size()/all_prod(): 要素数・列全体の積。O(1)。添字は0始まり。
// opは結合的、e()は単位元。opとSのコピー・ムーブ・代入は非送出。
// 重複キーは要素固有IDで整列し、方向に合わせてIDも昇順／降順。IDはsetでも保持。
// 構築とq操作の全体時間はO(1+(n+q)(W+log(n+1)))、W=64。追加領域はO(n+1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_SORT_AGGREGATION_DEFINED
#define SIRKIT_RANGE_SORT_AGGREGATION_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t CheckedRangeSortAggregationBase(std::size_t n, std::size_t limit)
{
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("RangeSortAggregation: too many items");
    }
    const auto base = std::bit_ceil(std::max(n, std::size_t{1}));
    if (base > limit)
    {
        throw std::length_error("RangeSortAggregation: too many segment tree leaves");
    }
    return base;
}
}

template <class S, auto op, auto e>
class RangeSortAggregation
{
    static_assert(std::is_nothrow_invocable_r_v<S, decltype(op), const S &, const S &>);
    static_assert(std::is_nothrow_copy_constructible_v<S> && std::is_nothrow_move_constructible_v<S> &&
                  std::is_nothrow_copy_assignable_v<S> && std::is_nothrow_move_assignable_v<S>);

public:
    struct Item
    {
        std::int32_t key;
        S value;
    };

    explicit RangeSortAggregation(std::span<const Item> items = {}) : identity_(std::invoke(e))
    {
        base_ = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedRangeSortAggregationBase(items.size(), tree_.max_size() / 2);
        n_ = static_cast<int>(items.size());
        nodes_.reserve(2 * items.size() + 1);
        nodes_.push_back({0, 0, 0, 0, 0, identity_, identity_});
        recycled_.reserve(items.size());
        tree_.assign(2 * base_, identity_);
        for (int i = 0; i < n_; ++i)
        {
            const auto &item = items[i];
            const auto node = static_cast<Index>(nodes_.size());
            nodes_.push_back({Pack(item.key, i), 0, 0, 1, 0, item.value, item.value});
            runs_.emplace_hint(runs_.end(), i, Run{node, false});
            tree_[base_ + i] = item.value;
        }
        for (auto i = base_; --i != 0;)
        {
            tree_[i] = Combine(tree_[2 * i], tree_[2 * i + 1]);
        }
    }

    RangeSortAggregation(const RangeSortAggregation &other)
        : identity_(other.identity_), nodes_(other.nodes_), recycled_(other.recycled_), tree_(other.tree_),
          runs_(other.runs_), n_(other.n_), base_(other.base_)
    {
        nodes_.reserve(2 * static_cast<std::size_t>(n_) + 1);
        recycled_.reserve(n_);
    }

    RangeSortAggregation &operator=(const RangeSortAggregation &other)
    {
        if (this != &other)
        {
            RangeSortAggregation next(other);
            Swap(next);
        }
        return *this;
    }

    RangeSortAggregation(RangeSortAggregation &&other) noexcept
        : identity_(other.identity_), nodes_(std::move(other.nodes_)), recycled_(std::move(other.recycled_)),
          tree_(std::move(other.tree_)), runs_(std::move(other.runs_)), n_(std::exchange(other.n_, 0)),
          base_(std::exchange(other.base_, 1))
    {
    }

    RangeSortAggregation &operator=(RangeSortAggregation &&other) noexcept
    {
        if (this != &other)
        {
            RangeSortAggregation next(std::move(other));
            Swap(next);
        }
        return *this;
    }

    [[nodiscard]] int size() const noexcept
    {
        return n_;
    }

    [[nodiscard]] S all_prod() const noexcept
    {
        return n_ == 0 ? identity_ : tree_[1];
    }

    void sort(int left, int right, bool descending = false)
    {
        CheckRange(left, right);
        if (right - left <= 1)
        {
            return;
        }
        const auto first = SplitAt(left);
        const auto last = SplitAt(right);
        auto root = first->second.root;
        for (auto it = std::next(first); it != last;)
        {
            root = Join(root, it->second.root);
            UpdateAggregate(it->first, identity_);
            it = runs_.erase(it);
        }
        first->second = {root, descending};
        UpdateAggregate(left, Whole(root, descending));
    }

    void set(int index, std::int32_t key, S value)
    {
        CheckIndex(index);
        const auto position = SplitAt(index);
        SplitAt(index + 1);
        auto &leaf = nodes_[position->second.root];
        leaf.key = Pack(key, static_cast<Index>(leaf.key));
        leaf.forward = leaf.backward = value;
        UpdateAggregate(index, value);
    }

    [[nodiscard]] Item get(int index) const
    {
        CheckIndex(index);
        const auto run = std::prev(runs_.upper_bound(index));
        auto node = run->second.root;
        int rank = index - run->first;
        if (run->second.descending)
        {
            rank = nodes_[node].count - rank - 1;
        }
        while (nodes_[node].height != 0)
        {
            const auto &parent = nodes_[node];
            const int left_size = nodes_[parent.left].count;
            if (rank < left_size)
            {
                node = parent.left;
            }
            else
            {
                rank -= left_size;
                node = parent.right;
            }
        }
        const auto &leaf = nodes_[node];
        return {Unpack(leaf.key), leaf.forward};
    }

    [[nodiscard]] S prod(int left, int right) const
    {
        CheckRange(left, right);
        if (left == right)
        {
            return identity_;
        }
        const auto first = std::prev(runs_.upper_bound(left));
        const auto last = std::prev(runs_.upper_bound(right - 1));
        if (first == last)
        {
            return Fold(first->second.root, left - first->first, right - first->first, first->second.descending);
        }
        const auto start = Fold(first->second.root, left - first->first, nodes_[first->second.root].count,
                                first->second.descending);
        const auto middle = Aggregate(std::next(first)->first, last->first);
        const auto end = Fold(last->second.root, 0, right - last->first, last->second.descending);
        return Combine(Combine(start, middle), end);
    }

private:
    using Index = std::uint32_t;

    struct Node
    {
        std::uint64_t key;
        Index left, right;
        int count, height;
        S forward, backward;
    };

    struct Run
    {
        Index root;
        bool descending;
    };

    S identity_;
    std::vector<Node> nodes_;
    std::vector<Index> recycled_;
    std::vector<S> tree_;
    std::map<int, Run> runs_;
    int n_ = 0;
    std::size_t base_ = 1;

    void Swap(RangeSortAggregation &other) noexcept
    {
        using std::swap;
        swap(identity_, other.identity_);
        nodes_.swap(other.nodes_);
        recycled_.swap(other.recycled_);
        tree_.swap(other.tree_);
        runs_.swap(other.runs_);
        swap(n_, other.n_);
        swap(base_, other.base_);
    }

    void CheckIndex(int index) const
    {
        (void)index;
    }

    void CheckRange(int left, int right) const
    {
        (void)left;
        (void)right;
    }

    static std::uint64_t Pack(std::int32_t key, Index id) noexcept
    {
        return (std::uint64_t{static_cast<Index>(key) ^ 0x80000000U} << 32) | id;
    }

    static std::int32_t Unpack(std::uint64_t key) noexcept
    {
        return std::bit_cast<std::int32_t>(static_cast<Index>(key >> 32) ^ 0x80000000U);
    }

    static S Combine(const S &left, const S &right) noexcept
    {
        return std::invoke(op, left, right);
    }

    [[nodiscard]] const S &Whole(Index node, bool descending) const noexcept
    {
        return descending ? nodes_[node].backward : nodes_[node].forward;
    }

    void Pull(Index index) noexcept
    {
        auto &node = nodes_[index];
        const auto &left = nodes_[node.left];
        const auto &right = nodes_[node.right];
        node.key = left.key;
        node.count = left.count + right.count;
        node.forward = Combine(left.forward, right.forward);
        node.backward = Combine(right.backward, left.backward);
    }

    Index Branch(Index left, Index right, int height) noexcept
    {
        const Node new_node{0, left, right, 0, height, identity_, identity_};
        Index result = 0;
        if (recycled_.empty())
        {
            result = static_cast<Index>(nodes_.size());
            nodes_.push_back(new_node);
        }
        else
        {
            result = recycled_.back();
            recycled_.pop_back();
            nodes_[result] = new_node;
        }
        Pull(result);
        return result;
    }

    // いずれも非空で、要素IDにより共通キーを持たないトライを結合する。
    Index Join(Index a, Index b) noexcept
    {
        const int separating_bit = std::bit_width(nodes_[a].key ^ nodes_[b].key);
        if (separating_bit > std::max(nodes_[a].height, nodes_[b].height))
        {
            if (nodes_[a].key > nodes_[b].key)
            {
                std::swap(a, b);
            }
            return Branch(a, b, separating_bit);
        }

        if (nodes_[a].height < nodes_[b].height)
        {
            std::swap(a, b);
        }
        auto &head = nodes_[a];
        if (head.height == nodes_[b].height)
        {
            const auto left = nodes_[b].left;
            const auto right = nodes_[b].right;
            recycled_.push_back(b);
            head.left = Join(head.left, left);
            head.right = Join(head.right, right);
        }
        else if ((nodes_[b].key >> (head.height - 1)) & 1)
        {
            head.right = Join(head.right, b);
        }
        else
        {
            head.left = Join(head.left, b);
        }
        Pull(a);
        return a;
    }

    // 0<rank<count。境界の枝を1個解放し、残りを再利用する。
    std::pair<Index, Index> Split(Index root, int rank) noexcept
    {
        auto &node = nodes_[root];
        const int left_size = nodes_[node.left].count;
        if (rank == left_size)
        {
            recycled_.push_back(root);
            return {node.left, node.right};
        }
        if (rank < left_size)
        {
            const auto [left, right] = Split(node.left, rank);
            node.left = right;
            Pull(root);
            return {left, root};
        }

        const auto [left, right] = Split(node.right, rank - left_size);
        node.right = left;
        Pull(root);
        return {root, right};
    }

    typename std::map<int, Run>::iterator SplitAt(int index)
    {
        if (index == n_)
        {
            return runs_.end();
        }
        const auto before = std::prev(runs_.upper_bound(index));
        if (before->first == index)
        {
            return before;
        }

        auto &run = before->second;
        // mapの確保後にトライを分割する。確保失敗なら元の区間を維持する。
        const auto after = runs_.emplace_hint(std::next(before), index, Run{0, run.descending});
        const int prefix = index - before->first;
        const int rank = run.descending ? nodes_[run.root].count - prefix : prefix;
        auto [left, right] = Split(run.root, rank);
        if (run.descending)
        {
            std::swap(left, right);
        }
        run.root = left;
        after->second.root = right;
        UpdateAggregate(before->first, Whole(left, run.descending));
        UpdateAggregate(index, Whole(right, run.descending));
        return after;
    }

    S Fold(Index index, int left, int right, bool descending) const noexcept
    {
        const auto &node = nodes_[index];
        if (left == 0 && right == node.count)
        {
            return Whole(index, descending);
        }
        const auto first = descending ? node.right : node.left;
        const auto last = descending ? node.left : node.right;
        const int middle = nodes_[first].count;
        if (right <= middle)
        {
            return Fold(first, left, right, descending);
        }
        if (left >= middle)
        {
            return Fold(last, left - middle, right - middle, descending);
        }
        return Combine(Fold(first, left, middle, descending), Fold(last, 0, right - middle, descending));
    }

    void UpdateAggregate(int index, const S &value) noexcept
    {
        auto position = base_ + static_cast<std::size_t>(index);
        tree_[position] = value;
        for (position >>= 1; position != 0; position >>= 1)
        {
            tree_[position] = Combine(tree_[2 * position], tree_[2 * position + 1]);
        }
    }

    S Aggregate(int left, int right) const noexcept
    {
        S prefix = identity_, suffix = identity_;
        auto first = base_ + static_cast<std::size_t>(left);
        auto last = base_ + static_cast<std::size_t>(right);
        for (; first < last; first >>= 1, last >>= 1)
        {
            if (first & 1)
            {
                prefix = Combine(prefix, tree_[first++]);
            }
            if (last & 1)
            {
                suffix = Combine(tree_[--last], suffix);
            }
        }
        return Combine(prefix, suffix);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
