// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Rollback_Lazy_Segment_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/rollback_lazy_segment_tree.md
//
// API一覧
// RollbackLazySegmentTree<S,F,op,e,mapping,composition,can_skip>(n/values): n個のe()または初期配列。O(n)時間・領域。
// snapshot(): 現在の変更履歴の長さを返す。O(1)。rollback(s): 現在の履歴上のsまで取り消す。O(取り消すノード数)。
// apply(l,r,f): [l,r)に作用をかける。set(k,x): 1点を代入。O(log(n+1))時間・追加履歴。
// prod(l,r): [l,r)の集約。get(k): 1点取得。O(log(n+1))時間。all_prod()/size(): 全体集約/要素数、O(1)。
// 照会はconstで変更履歴を増やさない。clear_history(): 現在の値を保って履歴を破棄する。
// mapping(f,s,length): 区間の集約sを作用fで変換。composition(f,g): gの後にfを適用。
// can_skip(f,s,length)は省略可。trueなら区間内の各要素への作用が恒等。添字は0始まりの半開区間。
// S,Fはコピー構築可能でムーブ構築・ムーブ代入が非送出。破棄した履歴からのsnapshotは使用不可。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ROLLBACK_LAZY_SEGMENT_TREE_DEFINED
#define SIRKIT_ROLLBACK_LAZY_SEGMENT_TREE_DEFINED

template <class S, class F, auto op, auto identity, auto mapping, auto composition, auto can_skip = nullptr>
class RollbackLazySegmentTree
{
    struct Node
    {
        S value;
        std::optional<F> lazy;
    };
    static_assert(std::is_nothrow_move_constructible_v<Node>);
    static_assert(std::is_nothrow_move_assignable_v<Node>);

    struct Change
    {
        std::size_t index;
        Node previous;
    };

public:
    using Snapshot = std::size_t;

    explicit RollbackLazySegmentTree(std::size_t n = 0)
        : n_(n), nodes_(Allocate(n))
    {
    }

    explicit RollbackLazySegmentTree(std::span<const S> values)
        : RollbackLazySegmentTree(values.size())
    {
        if (!values.empty())
        {
            Build(0, 0, n_, values);
        }
    }

    RollbackLazySegmentTree(const RollbackLazySegmentTree &) = default;
    RollbackLazySegmentTree &operator=(const RollbackLazySegmentTree &other)
    {
        RollbackLazySegmentTree copy(other);
        Swap(copy);
        return *this;
    }

    RollbackLazySegmentTree(RollbackLazySegmentTree &&other) noexcept
        : n_(std::exchange(other.n_, 0)), nodes_(std::move(other.nodes_)), history_(std::move(other.history_))
    {
        other.history_.clear();
    }

    RollbackLazySegmentTree &operator=(RollbackLazySegmentTree &&other) noexcept
    {
        RollbackLazySegmentTree moved(std::move(other));
        Swap(moved);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return n_;
    }

    [[nodiscard]] Snapshot snapshot() const noexcept
    {
        return history_.size();
    }

    void rollback(Snapshot mark)
    {
        if (mark > history_.size())
        {
            throw std::out_of_range("RollbackLazySegmentTree: snapshot");
        }
        Undo(mark);
    }

    void clear_history() noexcept
    {
        history_.clear();
    }

    void apply(std::size_t left, std::size_t right, const F &action)
    {
        CheckRange(left, right);
        if (left == right)
        {
            return;
        }
        const auto mark = snapshot();
        try
        {
            Apply(0, 0, n_, left, right, action, std::nullopt);
        }
        catch (...)
        {
            Undo(mark);
            throw;
        }
    }

    void set(std::size_t index, const S &value)
    {
        CheckIndex(index);
        const auto mark = snapshot();
        try
        {
            Set(0, 0, n_, index, value, std::nullopt);
        }
        catch (...)
        {
            Undo(mark);
            throw;
        }
    }

    [[nodiscard]] S all_prod() const
    {
        return n_ == 0 ? std::invoke(identity) : nodes_[0].value;
    }

    [[nodiscard]] S prod(std::size_t left, std::size_t right) const
    {
        CheckRange(left, right);
        return left == right ? std::invoke(identity) : Product(0, 0, n_, left, right, std::nullopt);
    }

    [[nodiscard]] S get(std::size_t index) const
    {
        CheckIndex(index);
        return Product(0, 0, n_, index, index + 1, std::nullopt);
    }

private:
    std::size_t n_;
    std::vector<Node> nodes_;
    std::vector<Change> history_;

    static std::vector<Node> Allocate(std::size_t n)
    {
        std::vector<Node> nodes;
        if (n > nodes.max_size() / 2)
        {
            throw std::length_error("RollbackLazySegmentTree: too many elements");
        }
        return n == 0 ? std::vector<Node>{} : std::vector<Node>(2 * n - 1, Node{std::invoke(identity), std::nullopt});
    }

    void Swap(RollbackLazySegmentTree &other) noexcept
    {
        std::swap(n_, other.n_);
        nodes_.swap(other.nodes_);
        history_.swap(other.history_);
    }

    void CheckIndex([[maybe_unused]] std::size_t index) const
    {
    }

    void CheckRange([[maybe_unused]] std::size_t left, [[maybe_unused]] std::size_t right) const
    {
    }

    void Build(std::size_t node, std::size_t begin, std::size_t end, std::span<const S> values)
    {
        if (end - begin == 1)
        {
            nodes_[node].value = S(values[begin]);
            return;
        }
        const auto mid = begin + (end - begin) / 2;
        const auto b = node + 2 * (mid - begin);
        Build(node + 1, begin, mid, values);
        Build(b, mid, end, values);
        nodes_[node].value = std::invoke(op, nodes_[node + 1].value, nodes_[b].value);
    }

    void Save(std::size_t index, Node after)
    {
        history_.push_back({index, nodes_[index]});
        nodes_[index] = std::move(after);
    }

    void Undo(Snapshot mark) noexcept
    {
        while (history_.size() > mark)
        {
            auto &last = history_.back();
            nodes_[last.index] = std::move(last.previous);
            history_.pop_back();
        }
    }

    static bool CanSkip(const F &action, const S &value, std::size_t length)
    {
        if constexpr (std::is_same_v<decltype(can_skip), std::nullptr_t>)
        {
            return false;
        }
        else
        {
            return std::invoke(can_skip, action, value, length);
        }
    }

    void ApplyWhole(std::size_t node, std::size_t length, const F &f)
    {
        const auto &previous = nodes_[node];
        auto result = std::invoke(mapping, f, previous.value, length);
        std::optional<F> tag;
        if (length > 1)
        {
            tag.emplace(previous.lazy ? std::invoke(composition, f, *previous.lazy) : f);
        }
        Save(node, Node{std::move(result), std::move(tag)});
    }

    static std::optional<F> Pending(const std::optional<F> &newer, const std::optional<F> &older)
    {
        if (newer && older)
        {
            return std::invoke(composition, *newer, *older);
        }
        return newer ? newer : older;
    }

    void KeepChild(std::size_t node, std::size_t length, const std::optional<F> &tag)
    {
        if (tag && !CanSkip(*tag, nodes_[node].value, length))
        {
            ApplyWhole(node, length, *tag);
        }
    }

    void Pull(std::size_t node, std::size_t right_child)
    {
        auto value = std::invoke(op, nodes_[node + 1].value, nodes_[right_child].value);
        Save(node, Node{std::move(value), std::nullopt});
    }

    // 遅延作用を引き継いだ子に対し、途中の状態を履歴へ追加せず一度だけ書く。
    void Apply(std::size_t node, std::size_t begin, std::size_t end,
               std::size_t left, std::size_t right, const F &f, const std::optional<F> &pending)
    {
        const auto length = end - begin;
        if (left <= begin && end <= right)
        {
            const auto action = pending ? std::invoke(composition, f, *pending) : f;
            if (!CanSkip(action, nodes_[node].value, length))
            {
                ApplyWhole(node, length, action);
            }
            return;
        }
        if (!pending && CanSkip(f, nodes_[node].value, length))
        {
            return;
        }
        const auto mid = begin + length / 2;
        const auto b = node + 2 * (mid - begin);
        const auto child_tag = Pending(pending, nodes_[node].lazy);
        if (left < mid)
        {
            Apply(node + 1, begin, mid, left, right, f, child_tag);
        }
        else
        {
            KeepChild(node + 1, mid - begin, child_tag);
        }
        if (right > mid)
        {
            Apply(b, mid, end, left, right, f, child_tag);
        }
        else
        {
            KeepChild(b, end - mid, child_tag);
        }
        Pull(node, b);
    }

    void Set(std::size_t node, std::size_t begin, std::size_t end, std::size_t index,
             const S &value, const std::optional<F> &pending)
    {
        if (end - begin == 1)
        {
            Save(node, Node{value, std::nullopt});
            return;
        }
        const auto mid = begin + (end - begin) / 2;
        const auto b = node + 2 * (mid - begin);
        const auto child_tag = Pending(pending, nodes_[node].lazy);
        if (index < mid)
        {
            Set(node + 1, begin, mid, index, value, child_tag);
            KeepChild(b, end - mid, child_tag);
        }
        else
        {
            KeepChild(node + 1, mid - begin, child_tag);
            Set(b, mid, end, index, value, child_tag);
        }
        Pull(node, b);
    }

    [[nodiscard]] S Product(std::size_t node, std::size_t begin, std::size_t end,
                            std::size_t left, std::size_t right, const std::optional<F> &pending) const
    {
        const auto &current = nodes_[node];
        if (left == begin && right == end)
        {
            return pending ? std::invoke(mapping, *pending, current.value, end - begin) : current.value;
        }
        const auto mid = begin + (end - begin) / 2;
        const auto b = node + 2 * (mid - begin);
        const auto child_tag = Pending(pending, current.lazy);
        if (right <= mid)
        {
            return Product(node + 1, begin, mid, left, right, child_tag);
        }
        if (left >= mid)
        {
            return Product(b, mid, end, left, right, child_tag);
        }
        return std::invoke(op, Product(node + 1, begin, mid, left, mid, child_tag),
                           Product(b, mid, end, mid, right, child_tag));
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
